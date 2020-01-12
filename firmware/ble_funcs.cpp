#include "ble_funcs.hpp"
// NOTE: The vast majority of this file comes from the SparkFun Apollo3 SDK Example8_BLE_LED source. It has been lightly modified and cleaned up to fit better into the Foxie Clock source code.

#include "Arduino.h"
#include "rtc_hal.hpp"

#define DEBUG
#define SERIAL_PORT Serial
#define BLE_PERIPHERAL_NAME "Foxie Clock"

#include <stdint.h>
#include <stdbool.h>

extern "C" {
    #include "wsf_types.h"
    #include "wsf_trace.h"
    #include "wsf_buf.h"

    #include "hci_handler.h"
    #include "dm_handler.h"
    #include "l2c_handler.h"
    #include "att_handler.h"
    #include "smp_handler.h"
    #include "l2c_api.h"
    #include "att_api.h"
    #include "smp_api.h"
    #include "app_api.h"
    #include "hci_core.h"
    #include "hci_drv.h"
    #include "hci_drv_apollo.h"
    #include "hci_drv_apollo3.h"

    #include "am_mcu_apollo.h"
    #include "am_util.h"

    #include "nus_api.h"
    #include "app_ui.h"

    #include "wsf_msg.h"
}

//*****************************************************************************
//
// Forward declarations.
//
//*****************************************************************************
void exactle_stack_init(void);
void scheduler_timer_init(void);
void set_next_wakeup(void);
void setAdvName(const char* str);
extern "C" void set_adv_name( const char* str );

// ****************************************
// 
// C-callable led functions
// 
// ****************************************

enum AlertCommands_e
{
    CMD_SET_TIME = 0x10,
};

enum AlertState_e
{
    STATE_WAIT,
    STATE_SET_TIME,
};
void handleSetTimeState(uint8_t val);

enum ClockSetState_e
{
    CS_HOUR,
    CS_MINUTE,
    CS_SECOND,
};


AlertState_e g_alertState = STATE_WAIT;

extern "C" void alert(uint8_t val)
{
    switch (g_alertState)
    {
    case STATE_WAIT:
        if (val == CMD_SET_TIME)
        {
            g_alertState = STATE_SET_TIME;
        }
        break;

    case STATE_SET_TIME:
        handleSetTimeState(val);

    default:
        break;
    }

    // TODO: Implement a timeout for this
}

extern void updateClock(bool force = false);
void handleSetTimeState(uint8_t val)
{
    static ClockSetState_e state = CS_HOUR;

    static int h, m;

    switch (state)
    {
    case CS_HOUR:
        h = val;
        state = CS_MINUTE;
        break;

    case CS_MINUTE:
        m = val;
        state = CS_SECOND;
        break;

    case CS_SECOND:
        rtc_hal_setTime(h, m, val + 1);
        state = CS_HOUR;
        g_alertState = STATE_WAIT;
        break;

    default:
        break;
    }
    updateClock(true);
}

void ble_init() {
  set_adv_name(BLE_PERIPHERAL_NAME);

  HciDrvRadioBoot(0);

  exactle_stack_init();

  // NUS = "nordic uart service"
  NusStart();
}

//*****************************************************************************
//
// Timer configuration macros.
//
//*****************************************************************************
#define MS_PER_TIMER_TICK           10  // Milliseconds per WSF tick
#define CLK_TICKS_PER_WSF_TICKS     5   // Number of CTIMER counts per WSF tick.

//*****************************************************************************
//
// WSF buffer pools.
//
//*****************************************************************************
#define WSF_BUF_POOLS               4

// Important note: the size of g_pui32BufMem should includes both overhead of internal
// buffer management structure, wsfBufPool_t (up to 16 bytes for each pool), and pool
// description (e.g. g_psPoolDescriptors below).

// Memory for the buffer pool
static uint32_t g_pui32BufMem[(WSF_BUF_POOLS*16
         + 16*8 + 32*4 + 64*6 + 280*8) / sizeof(uint32_t)];

// Default pool descriptor.
static wsfBufPoolDesc_t g_psPoolDescriptors[WSF_BUF_POOLS] =
{
    {  16,  8 },
    {  32,  4 },
    {  64,  6 },
    { 280,  8 }
};


//*****************************************************************************
//
// Tracking variable for the scheduler timer.
//
//*****************************************************************************
uint32_t g_ui32LastTime = 0;
extern "C" void radio_timer_handler(void);


//*****************************************************************************
//
// Initialization for the ExactLE stack.
//
//*****************************************************************************
void exactle_stack_init(void){
    wsfHandlerId_t handlerId;

    //
    // Set up timers for the WSF scheduler.
    //
    scheduler_timer_init();
    WsfTimerInit();

    //
    // Initialize a buffer pool for WSF dynamic memory needs.
    //
    WsfBufInit(sizeof(g_pui32BufMem), (uint8_t*)g_pui32BufMem, WSF_BUF_POOLS, g_psPoolDescriptors);

    //
    // Initialize security.
    //
    SecInit();
    SecAesInit();
    SecCmacInit();
    SecEccInit();

    //
    // Set up callback functions for the various layers of the ExactLE stack.
    //
    handlerId = WsfOsSetNextHandler(HciHandler);
    HciHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(DmHandler);
    DmDevVsInit(0);
    DmAdvInit();
    DmConnInit();
    DmConnSlaveInit();
    DmSecInit();
    DmSecLescInit();
    DmPrivInit();
    DmHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(L2cSlaveHandler);
    L2cSlaveHandlerInit(handlerId);
    L2cInit();
    L2cSlaveInit();

    handlerId = WsfOsSetNextHandler(AttHandler);
    AttHandlerInit(handlerId);
    AttsInit();
    AttsIndInit();
    AttcInit();

    handlerId = WsfOsSetNextHandler(SmpHandler);
    SmpHandlerInit(handlerId);
    SmprInit();
    SmprScInit();
    HciSetMaxRxAclLen(251);

    handlerId = WsfOsSetNextHandler(AppHandler);
    AppHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(NusHandler);
    NusHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(HciDrvHandler);
    HciDrvHandlerInit(handlerId);
}

//*****************************************************************************
//
// Set up a pair of timers to handle the WSF scheduler.
//
//*****************************************************************************
void
scheduler_timer_init(void)
{
    //
    // One of the timers will run in one-shot mode and provide interrupts for
    // scheduled events.
    //
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERA,
                                (AM_HAL_CTIMER_INT_ENABLE |
                                 AM_HAL_CTIMER_LFRC_512HZ |
                                 AM_HAL_CTIMER_FN_ONCE));

    //
    // The other timer will run continuously and provide a constant time-base.
    //
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERB);
    am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERB,
                                 (AM_HAL_CTIMER_LFRC_512HZ |
                                 AM_HAL_CTIMER_FN_CONTINUOUS));

    //
    // Start the continuous timer.
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERB);

    //
    // Enable the timer interrupt.
    //
    am_hal_ctimer_int_register(AM_HAL_CTIMER_INT_TIMERA0, radio_timer_handler);
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);
    NVIC_EnableIRQ(CTIMER_IRQn);
}

//*****************************************************************************
//
// Calculate the elapsed time, and update the WSF software timers.
//
//*****************************************************************************
void
ble_update_scheduler_timers(void)
{
    uint32_t ui32CurrentTime, ui32ElapsedTime;

    //
    // Read the continuous timer.
    //
    ui32CurrentTime = am_hal_ctimer_read(0, AM_HAL_CTIMER_TIMERB);

    //
    // Figure out how long it has been since the last time we've read the
    // continuous timer. We should be reading often enough that we'll never
    // have more than one overflow.
    //
    ui32ElapsedTime = (ui32CurrentTime >= g_ui32LastTime ?
                       (ui32CurrentTime - g_ui32LastTime) :
                       (0x10000 + ui32CurrentTime - g_ui32LastTime));

    //
    // Check to see if any WSF ticks need to happen.
    //
    if ( (ui32ElapsedTime / CLK_TICKS_PER_WSF_TICKS) > 0 )
    {
        //
        // Update the WSF timers and save the current time as our "last
        // update".
        //
        WsfTimerUpdate(ui32ElapsedTime / CLK_TICKS_PER_WSF_TICKS);

        g_ui32LastTime = ui32CurrentTime;
    }

    wsfOsDispatcher();
    set_next_wakeup();
}

//*****************************************************************************
//
// Set a timer interrupt for the next upcoming scheduler event.
//
//*****************************************************************************
void
set_next_wakeup(void)
{
    bool_t bTimerRunning;
    wsfTimerTicks_t xNextExpiration;

    //
    // Stop and clear the scheduling timer.
    //
    am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);

    //
    // Check to see when the next timer expiration should happen.
    //
    xNextExpiration = WsfTimerNextExpiration(&bTimerRunning);

    //
    // If there's a pending WSF timer event, set an interrupt to wake us up in
    // time to service it. Otherwise, set an interrupt to wake us up in time to
    // prevent a double-overflow of our continuous timer.
    //
    if ( xNextExpiration )
    {
        am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA,
                                 xNextExpiration * CLK_TICKS_PER_WSF_TICKS, 0);
    }
    else
    {
        am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, 0x8000, 0);
    }

    //
    // Start the scheduling timer.
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);
}

//*****************************************************************************
//
// Interrupt handler for the CTIMERs
//
//*****************************************************************************
extern "C" void radio_timer_handler(void){
    // Signal radio task to run

    WsfTaskSetReady(0, 0);
}

//*****************************************************************************
//
// Interrupt handler for the CTIMERs
//
//*****************************************************************************
extern "C" void am_ctimer_isr(void){
    uint32_t ui32Status;

    //
    // Check and clear any active CTIMER interrupts.
    //
    ui32Status = am_hal_ctimer_int_status_get(true);
    am_hal_ctimer_int_clear(ui32Status);

    //
    // Run handlers for the various possible timer events.
    //
    am_hal_ctimer_int_service(ui32Status);
}

//*****************************************************************************
//
// Interrupt handler for BLE
//
//*****************************************************************************
extern "C" void am_ble_isr(void){
    HciDrvIntService();
}

// ****************************************
// 
// Debug print functions
// 
// ****************************************
#define DEBUG_UART_BUF_LEN 256

extern "C" void debug_print(const char* f, const char* F, uint16_t L){
  SERIAL_PORT.printf("fm: %s, file: %s, line: %d\n", f, F, L);
}

extern "C" void debug_printf(char* fmt, ...){
#ifdef DEBUG
    char    debug_buffer        [DEBUG_UART_BUF_LEN];
    va_list args;
    va_start (args, fmt);
    vsnprintf(debug_buffer, DEBUG_UART_BUF_LEN, (const char*)fmt, args);
    va_end (args);

    SERIAL_PORT.print(debug_buffer);
#endif //DEBUG  
}

