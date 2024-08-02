#include "main.h"

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC2_Init(void);

#define I2C_ADDR 0x68
#define I2C_CASE_ADC_0 0x00
#define I2C_CASE_ADC_1 0x01
#define I2C_CASE_GPIOS 0x02
#define I2C_CASE_ISO1H816G_POLARITY 0x03
#define I2C_CASE_ISO1H816G_LEVEL 0x04

uint8_t i2c_buff[10];

int main(void) {
    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_SPI1_Init();
    MX_ADC2_Init();

    while (1) {
        if (HAL_I2C_Master_Receive_IT(&hi2c1, I2C_ADDR << 1, i2c_buff, sizeof(i2c_buff)) != HAL_OK) {
            Error_Handler();
        }

        HAL_Delay(1000);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    uint8_t i2c_data;

    uint8_t adc1_value;
    uint8_t adc2_value;
    uint8_t gpio_state;
    uint8_t polarity;
    uint8_t level;

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc1_value = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
    adc2_value = HAL_ADC_GetValue(&hadc2);

    gpio_state = 0x00;
    gpio_state |= HAL_GPIO_ReadPin(GPIOA, IN_0_Pin);
    gpio_state |= (HAL_GPIO_ReadPin(GPIOA, IN_1_Pin)) << 1;
    gpio_state |= (HAL_GPIO_ReadPin(GPIOA, IN_2_Pin)) << 2;
    gpio_state |= (HAL_GPIO_ReadPin(GPIOA, IN_3_Pin)) << 3;
    gpio_state |= (HAL_GPIO_ReadPin(GPIOA, IN_4_Pin)) << 4;
    gpio_state |= (HAL_GPIO_ReadPin(GPIOA, IN_5_Pin)) << 5;
    gpio_state |= (HAL_GPIO_ReadPin(GPIOB, IN_6_Pin)) << 6;
    gpio_state |= (HAL_GPIO_ReadPin(GPIOB, IN_7_Pin)) << 7;

    HAL_I2C_Master_Receive(hi2c, I2C_ADDR, &i2c_data, 1, HAL_MAX_DELAY);

    switch (i2c_data) {
        case I2C_CASE_ADC_0:
            HAL_I2C_Master_Transmit(hi2c, I2C_ADDR, (uint8_t *)&adc1_value, 2, HAL_MAX_DELAY);
            break;
        case I2C_CASE_ADC_1:
            HAL_I2C_Master_Transmit(hi2c, I2C_ADDR, (uint8_t *)&adc2_value, 2, HAL_MAX_DELAY);
            break;
        case I2C_CASE_GPIOS:
            HAL_I2C_Master_Transmit(hi2c, I2C_ADDR, &gpio_state, 1, HAL_MAX_DELAY);
            break;
        case I2C_CASE_ISO1H816G_POLARITY:
            HAL_I2C_Master_Receive(hi2c, I2C_ADDR, &polarity, 1, HAL_MAX_DELAY);
            HAL_SPI_Transmit(&hspi1, &polarity, 1, HAL_MAX_DELAY);
            break;
        case I2C_CASE_ISO1H816G_LEVEL:
            HAL_I2C_Master_Receive(hi2c, I2C_ADDR, &level, 1, HAL_MAX_DELAY);
            HAL_SPI_Transmit(&hspi1, &level, 1, HAL_MAX_DELAY);
            break;
        default:
            break;
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_ADC2_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc2.Instance = ADC2;
    hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc2.Init.ContinuousConvMode = DISABLE;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc2.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc2) != HAL_OK) {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_SPI1_Init(void) {
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = IN_7_Pin | IN_6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin =
        IN_5_Pin | IN_4_Pin | IN_3_Pin | IN_2_Pin | IN_1_Pin | IN_0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
