#include "lcd_widget_test_basic_indicator.h"
#include "lcd_widget_test_drivers_indicator.h"

static const char* label = "Drivers";
StatusBasic statusDrv;

void TestDrvDisplay_DrawAll(uint16_t baseX, uint16_t baseY)
{
	//test
	statusDrv.items[8] = true;
	statusDrv.items[9] = true;

	statusDrv.baseX = baseX;
	statusDrv.baseY = baseY;
	strcpy(statusDrv.label, label);

	TestBasicDisplay_DrawAll(&statusDrv);
}

void TestDrvDisplay_SetStatus(uint8_t index, bool status)
{
    if (index >= STATUS_COUNT) return;

    if (statusDrv.items[index] != status) {
    	statusDrv.items[index] = status;
        DrawStatusBox(index, &statusDrv);
    }
}
