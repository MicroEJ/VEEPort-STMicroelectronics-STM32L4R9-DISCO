/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
 */

#include "qb_impl.h"

/**
 * When qualification bundle is used (see qb_impl.h), the functions of this file are used to replace
 * qualification bundle CORE weak functions.
 */

#ifdef TEST_QUALIFICATION_BUNDLE

#include "x_ram_checks.h"
#include "../../../../framework/c/utils/inc/u_print.h"
#include "../../../../framework/c/utils/inc/u_time_base.h"
#include "time_hardware_timer.h"

/*
 * Declare SDRAM zone to test.
 */
X_RAM_CHECKS_zone_t sb_psram[] = 
{
  // PSRAM 
  {0x60000000, 0x60020000, 0, 0, 0},
};

/*
 * Declare "source" zone used to bench transfers with PSRAM.
 * Used the internal flash as source.
 */
X_RAM_CHECKS_zone_t qb_source[] = {{0x08000400, 0x08001400, 0, 0, 0}};

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get32bitSourceZone(void)
{
  return qb_source;
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get16bitSourceZone(void)
{
  return X_RAM_CHECKS_get32bitSourceZone();
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get8bitSourceZone(void)
{
  return X_RAM_CHECKS_get32bitSourceZone();
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get32bitZones(void)
{
  return sb_psram;
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get16bitZones(void)
{
  return X_RAM_CHECKS_get32bitZones();
}

X_RAM_CHECKS_zone_t* X_RAM_CHECKS_get8bitZones(void)
{
  return X_RAM_CHECKS_get32bitZones();
}

uint8_t X_RAM_CHECKS_get32bitZoneNumber(void)
{
	return sizeof(sb_psram) / sizeof(X_RAM_CHECKS_zone_t);
}

uint8_t X_RAM_CHECKS_get16bitZoneNumber(void)
{
	return X_RAM_CHECKS_get32bitZoneNumber();
}

uint8_t X_RAM_CHECKS_get8bitZoneNumber(void)
{
	return X_RAM_CHECKS_get32bitZoneNumber();
}

void UTIL_TIME_BASE_initialize(void)
{
	time_hardware_timer_initialize();
}

uint64_t UTIL_TIME_BASE_getTime(void)
{
	return time_hardware_timer_getTimeNanos() / 1000;
}

#endif // TEST_QUALIFICATION_BUNDLE
