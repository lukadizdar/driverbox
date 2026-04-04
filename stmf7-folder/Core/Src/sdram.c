#include "fmc.h"
#include "sdram.h"

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

void SDRAM_Init(void)
{
  FMC_SDRAM_CommandTypeDef command;

  /* Step 1: Configure a clock configuration enable command */
  command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand(&hsdram1, &command, 0xFFFF);

  /* Step 2: Insert 100us minimum delay */
  HAL_Delay(1);

  /* Step 3: Configure a PALL (precharge all) command */
  command.CommandMode = FMC_SDRAM_CMD_PALL;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand(&hsdram1, &command, 0xFFFF);

  /* Step 4: Configure Auto-Refresh command */
  command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  command.AutoRefreshNumber = 8;
  command.ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand(&hsdram1, &command, 0xFFFF);

  /* Step 5: Program the external memory mode register */
  command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition =
      SDRAM_MODEREG_BURST_LENGTH_1          |
      SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
      SDRAM_MODEREG_CAS_LATENCY_3           |
      SDRAM_MODEREG_OPERATING_MODE_STANDARD |
      SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  HAL_SDRAM_SendCommand(&hsdram1, &command, 0xFFFF);

  /* Step 6: Set the refresh rate counter
   * Refresh rate = (COUNT + 1) x SDRAM clock frequency
   * SDRAM clock = HCLK / 2 = 200MHz / 2 = 100MHz
   * Refresh period = 64ms / 4096 rows = 15.625us
   * COUNT = (15.625us x 100MHz) - 20 = 1542 */
  HAL_SDRAM_ProgramRefreshRate(&hsdram1, 1542);
}
