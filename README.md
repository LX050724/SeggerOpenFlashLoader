# SEGGER JLink 外置Flash下载算法

依赖库使用cmake下载，为节省下载时间驱动使能默认关闭，需要手动开启相应驱动，或使能`DRIVER_ALL_ENABLE`开启所有驱动

`BOARD_MANFACTURER`为选中的制造商列表，默认选中全部

`BOARD_NAME`为选中的开发板列表，默认选中全部

当前支持的开发板：

|制造商|型号|MCU型号|Flash型号|算法名称|地址|备注|
|---|---|---|---|---|---|---|
|正点原子|H743|STM32H743IIT6|W25Q256|Alientek_H743_W25Q256|0x90000000||
|达秒科技|MC02|STM32H723VGT6|W25Q64|DAMIAO_MC02_W25Q64|0x70000000||
|RTThread|ArtPi|STM32H750XBH6|W25Q64|RTThread_ArtPi_QSPI_W25Q64|0x90000000|程序Flash|
||||W25Q128|RTThread_ArtPi_SPI_W25Q128|*0xA0000000|数据Flash|
|None|FocG431Pico|STM32G431CBU6|GD25Q80|None_FocG431Pico_GD25Q80|*0xA0000000|数据Flash，待验证|

**：虚拟地址*

