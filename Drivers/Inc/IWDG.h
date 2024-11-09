
/** @brief How long we want to wait (in ms) before we refresh
 *  the IDWG to prevent it from resetting the system.
 *  Should be smaller than the refresh time of the system itself.
 */
int IWDG_REFRESH_TIME;


void IWDG_Init();
void IWDG_Start();
void IWDG_Reset();
void IWDG_CheckStatus();

void Error_Handler(void);