/*
 * VideoCore4_Drivers
 * Copyright (c) 2017 Kristina Brooks (C++)
 * Copyright (c) 2021 Anichang (plain C)
 *
 * BCM2708 power management driver.
 */

#include <drivers/iodevice.h>

static iodevice_t *g_DeviceRegistry[32] = { 0 };
static unsigned int g_NewDriverIndex = 0;

void driverLog(iodevice_t *dev, const char* fnName, const char* fmt, ...) {
  va_list va;
  uint32_t clock_lo = ST_CLO;

  printf("%3ld.%06ld ", clock_lo / 1000000, clock_lo % 1000000);

  printf("%s::%s(): ", dev->driverName, fnName);
  va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
	printf("\n");
}

void registerDriver(iodevice_t *dev) {
  g_DeviceRegistry[g_NewDriverIndex++] = dev;
  IODriverLog("driver registered on platform IO plane");
}

void inline setName(iodevice_t *dev, const char* name) {
	dev->driverName = name;
}

void inline setTag(iodevice_t *dev, uint32_t tag) {
	dev->tag = tag;
}

iodevice_t *findByTag(uint32_t tag) {
	for (unsigned int i = 0; i < (sizeof(g_DeviceRegistry) / sizeof(void*)); i++) {
		iodevice_t *dev = g_DeviceRegistry[i];
		if (dev && dev->tag == tag) {
			return dev;
		}
	}

	return nullptr;
}

void _beforeInit(iodevice_t *dev) {
	dev->tag = 0;
	dev->deviceState = kIODeviceUninitialized;
}

IODeviceState getDeviceState(iodevice_t *dev) {
	if (_getDeviceState) _getDeviceState();
	return iodev.deviceState;
}

void init(iodevice_t *dev) {
	if (_init) _init();
	else panic("IODevice objects have to override init()");
}

void _afterInit(iodevice_t *dev) {
	assert(dev->driverName);
	if (dev->tag)
		registerDriver(dev);
}

void start(iodevice_t *dev) {
	if (_start) _start();
	dev->deviceState = kIODeviceStarted;
}

void stop(iodevice_t *dev) {
	if (_stop) _stop();
	dev->deviceState = kIODeviceStopped;
}

// GPCLK

iodevice_t GPCLK(volatile uint32_t *ctrl_addr, volatile uint32_t *div_addr, const char *name) {
	iodevice_t dev;

	dev.ctrl = ctrl_addr;
	dev.divider = div_addr;
	setName(&dev, name);

	return dev;
}

iodevice_t GPCLK0 = GPCLK(&CM_GP0CTL, &CM_GP0DIV, "GPCLK0");
iodevice_t GPCLK1 = GPCLK(&CM_GP1CTL, &CM_GP1DIV, "GPCLK1");
iodevice_t GPCLK2 = GPCLK(&CM_GP2CTL, &CM_GP2DIV, "GPCLK2");

/*
 * VideoCore4_Drivers
 * Copyright (c) 2017 Kristina Brooks
 *
 * PLL VCOs and their channels.
 */

#include <drivers/BCM2708ClockDomains.hpp>

/*
PLL/Channel tree:
|-XOSC
 |-PLLA
   |-CORE
   |-PER
   |-CCP2
   |-DSI0
 |-PLLB
   |-ARM
     |-ARM Clock (CM_ARM*)
   |-SP0 (???)
   |-SP1 (???)
   |-SP2 (???)
 |-PLLC
   |-CORE0
   |-CORE1
   |-PER
   |-CORE2 (???)
 |-PLLD
   |-DSI0
   |-DSI1
   |-CORE
   |-PER
 |-PLLH
   |-AUX
   |-PIX
   |-RCAL
   |-STS
 */

/***********************************************************************
 *
 * Common VCO stuff.
 *
 ***********************************************************************/

void BCM2708VCO::setDigValues() {
	IODriverLog("setting DIG values for this VCO ...");

	uint32_t dig0 = dig[0],
		dig1 = dig[1],
		dig2 = dig[2],
		dig3 = dig[3];

	/* 0xAA covers all possible HOLD bits in CM_PLLx regs */
        // for PLLA: HOLDDSI0, HOLDCCP2, HOLDCORE, HOLDPER amd DIGRST
        // for PLLB: HOLDARM and DIGRST
        // for PLLC: HOLDCORE0, HOLDCORE1, HOLDCORE2, HOLDPER, DIGRST
        // for PLLD: HOLDDSI0, HOLDDSI1, HOLDCORE, HOLDPER, DIGRST
	*cmPllCtrl = CM_PASSWORD | 0xAA;

	dig[3] = A2W_PASSWORD | (dig3);
	dig[2] = A2W_PASSWORD | (dig2 & 0xFFEFFBFE);
	dig[1] = A2W_PASSWORD | (dig1 & ~(1 << 14));
	dig[0] = A2W_PASSWORD | (dig0);

	*ctrl = A2W_PASSWORD | 0x20000 | *ctrl; // set PRSTN bit

	dig3 |= 0x42;

	dig[3] = A2W_PASSWORD | (dig3);
	dig[2] = A2W_PASSWORD | (dig2);
	dig[1] = A2W_PASSWORD | (dig1);
	dig[0] = A2W_PASSWORD | (dig0);
}

void BCM2708VCO::dumpDigValues() {
	IODriverLog("DIG0: 0x%X, DIG1: 0x%X, DIG2: 0x%X, DIG3: 0x%X",
		dig[0],
		dig[1],
		dig[2],
		dig[3]);
}

void BCM2708VCO::configure(uint32_t ndiv, uint32_t frac, uint32_t pdiv, bool halve) {
  // based on bcm2835_pll_set_rate from the linux kernel
  uint32_t ana[4];
  bool do_early_ana_setup;

  for (int i=3; i>=0; i--) ana[i] = this->ana[i];
  bool was_using_prediv = ana[1] & prediv_mask;

  ana[0] &= ~ana0_mask;
  ana[0] |= ana0_set;

  ana[1] &= ~ana1_mask;
  ana[1] |= ana1_set;

  ana[3] &= ~ana3_mask;
  ana[3] |= ana3_set;

  if (was_using_prediv && !halve) {
    ana[1] &= ~prediv_mask;
    do_early_ana_setup = true;
  } else if (!was_using_prediv && halve) {
    ana[1] |= prediv_mask;
    do_early_ana_setup = false;
  } else do_early_ana_setup = true;

  // unmask the input clock to the PLL
  A2W_XOSC_CTRL |= A2W_PASSWORD | reference_enable;

  if (do_early_ana_setup) write_ana(ana);

  *(this->frac) = frac;

  uint32_t a2w_ctl = *ctrl;
  a2w_ctl &= ~(A2W_PLLC_CTRL_NDIV_SET | A2W_PLLC_CTRL_PDIV_SET); // clear all bits used by ndiv and pdiv
  a2w_ctl |= ndiv;
  a2w_ctl |= pdiv << A2W_PLLC_CTRL_PDIV_LSB;
  *ctrl = A2W_PASSWORD | a2w_ctl;

  if (!do_early_ana_setup) write_ana(ana);
}

void BCM2708VCO::write_ana(uint32_t ana[4]) {
  for (int i = 3; i >= 0; i--) this->ana[i] = A2W_PASSWORD | ana[i];
}

BCM2835PLLDivider::BCM2835PLLDivider(const char *name, volatile uint32_t *a2w, volatile uint32_t *cm, uint8_t channel) : a2w(a2w), cm(cm), channel(channel) {
  setName(name);
}

void BCM2835PLLDivider::configure(uint32_t divisor) {
  uint32_t a2w = *this->a2w;
  a2w &= ~A2W_PLLC_PER_DIV_SET;
  a2w |= divisor & A2W_PLLC_PER_DIV_SET;
  *this->a2w = A2W_PASSWORD | a2w;

  uint32_t cm = *this->cm;
  *this->cm = CM_PASSWORD | cm | (1 << (channel * 2));
  *this->cm = CM_PASSWORD | (cm & ~(1 << (channel * 2)));

  // A2W_PLLC_PER_CHENB_SET is either the enable or disable flag
  // name says enable, but linux source calls it disable and treats it as disable
}

/***********************************************************************
 *
 * PLL VCOs for specific PLLs.
 *
 ***********************************************************************/

struct BCM2708PLLB : BCM2708VCO {
	IODriverConstructor(BCM2708PLLB);

	/*
	 * Working: DIG0: 0xAAA030, DIG1: 0x403A, DIG2: 0x402401, DIG3: 0x2
	 */

	virtual void init() override {
		ctrl = RegToRef(A2W_PLLB_CTRL);
		ana = RegToRef(A2W_PLLB_ANA0);
		dig = RegToRef(A2W_PLLB_DIG0);
		cmPllCtrl = RegToRef(CM_PLLB);

		IODriverLog("PLLB VCO registered");

		setTag('PLLB');
	}
};
IODriverCreateSingletonInstance(BCM2708PLLB);

/***********************************************************************
 *
 * ARM Clock.
 *
 ***********************************************************************/

struct BCM2708ClockDomain : IODevice {
	volatile uint32_t* cmDiv;
	volatile uint32_t* cmCtrl;
};

struct BCM2708ArmClockDomain : BCM2708ClockDomain {

};

BCM2835PLLDivider pllc_per("PLLC_PER", &A2W_PLLC_PER, &CM_PLLC, 3);

/*
 * VideoCore4_Drivers
 * Copyright (c) 2017 Kristina Brooks
 *
 * BCM2708 power management driver.
 */

#include <drivers/BCM2708PowerManagement.hpp>

#include <stdio.h>

#define PM_UNK_CFG_CLR 0xFFFCFFFF

PowerManagementDomain* g_BCM2708PowerDomains[kCprPowerDomain_MAX];

PowerManagementDomain* PowerManagementDomain::getDeviceForDomain(cpr_power_domain_t domain) {
	if (domain > kCprPowerDomain_MAX-1) {
		return nullptr;
	}
	PowerManagementDomain* pm = g_BCM2708PowerDomains[domain];
        return pm;
}

/***********************************************************************
 *
 * Superclass for all power domains.
 *
 ***********************************************************************/

struct BCM2708PowerDomain : PowerManagementDomain {
	volatile uint32_t* pmReg;
	uint32_t rstnMask;
	cpr_power_domain_t powerDomain;

	inline void pmvOr(uint32_t& pmv, uint32_t value) {
		pmv |= value;
		*pmReg = pmv;
	}

	inline void pmvAnd(uint32_t& pmv, uint32_t value) {
		pmv &= value;
		*pmReg = pmv;
	}

	bool waitForPmBit(uint32_t bit) {
		for (int i = 0; i < 20; i++) {
			if (*pmReg & bit) {
				return true;
			}
			waitOscTicks();
		}
		return false;
	}

	cpr_power_result_t waitForPOWOK(uint32_t& pmv) {
		IODriverLog("waiting for POWOK ...");

		for (int i = 1; i < 5; i++) {
			if (!waitForPmBit(PM_GNRIC_POWOK_SET)) {
				/* only go up to 3 */
				if (i == 4) {
					IODriverLog("timed out waiting for powok!");
					return kCprPowOkTimeout;
				}

				pmv = (pmv & PM_UNK_CFG_CLR) | (i << PM_GNRIC_CFG_LSB);
				IODriverLog("timed out, trying a different cfg value: 0x%X", pmv);
				*pmReg = pmv;
			}
		}

		IODriverLog("got POWOK with CFG=0x%X", *pmReg & PM_GNRIC_CFG_SET);
		return kCprSuccess;
	}

	void beginPowerUpSequence(uint32_t& pmv) {
		IODriverLog("starting power up sequence ...");
		pmvOr(pmv, PM_GNRIC_POWUP_SET);
	}

	cpr_power_result_t completePowerUpSequence(uint32_t& pmv, bool setPowup = true) {
		pmvOr(pmv, PM_GNRIC_ISPOW_SET);
		pmvOr(pmv, PM_GNRIC_MEMREP_SET);

		IODriverLog("waiting for MRDONE ...");
		if (!waitForPmBit(PM_GNRIC_MRDONE_SET)) {
			IODriverLog("timed out waiting for MRDONE: 0x%X", *pmReg);
			return kCprMrDoneTimeout;
		}

		pmvOr(pmv, PM_GNRIC_ISFUNC_SET);

		return kCprSuccess;
	}

	virtual cpr_power_result_t powerOn() {
		uint32_t pmv;
		cpr_power_result_t res;

		IODriverLog("powering on (rstnMask=0x%X) ...", rstnMask);

		pmv = ((*pmReg & rstnMask) & PM_UNK_CFG_CLR) | PM_PASSWORD;
		*pmReg = pmv;
		udelay(15);

		beginPowerUpSequence(pmv);

		res = waitForPOWOK(pmv);
		if (res != kCprSuccess)
			return res;

		res = completePowerUpSequence(pmv);
		if (res != kCprSuccess)
			return res;

		IODriverLog("domain powered on succesfully");

		return kCprSuccess;
	}

	virtual void setReset() override {
		IODriverLog("setting RSTN bits to 0x%X ...", (~rstnMask));
		*pmReg |= (~rstnMask) | PM_PASSWORD;
		udelay(300);
	}

	void waitOscTicks(uint32_t ticks = 2) {
		CM_OSCCOUNT = CM_PASSWORD | ticks;
		while(CM_OSCCOUNT != 0);
	}

	template <typename T>
	void setPmReg(T regAddr) {
		pmReg = reinterpret_cast<volatile uint32_t*>(regAddr);
	}

	void registerPowerDomain(cpr_power_domain_t domain) {
		g_BCM2708PowerDomains[domain] = this;
	}
};

/***********************************************************************
 *
 * Image power domain.
 * Contains: PERII (USB), H264, ISP
 *
 ***********************************************************************/

struct BCM2708PowerDomainImage : BCM2708PowerDomain {
	IODriverConstructor(BCM2708PowerDomainImage);

	virtual void init() override {
		setName("BCM2708PowerDomainImage");
		setPmReg(&PM_IMAGE);
		registerPowerDomain(kCprPowerDomainImage);
		rstnMask = ~(PM_IMAGE_ISPRSTN_SET | PM_IMAGE_H264RSTN_SET | PM_IMAGE_PERIRSTN_SET);
	}

	virtual cpr_power_result_t powerOn() {
		uint32_t pmv;
		cpr_power_result_t res;

		IODriverLog("powering on, current PM_IMAGE state: 0x%X", PM_IMAGE);

		pmv = *pmReg | 0x10000 | PM_PASSWORD;
		*pmReg = pmv;

		/*
		 * first sequence has to be done with this CFG.
		 */
		beginPowerUpSequence(pmv);

		res = waitForPOWOK(pmv);
		if (res != kCprSuccess)
			return res;

		IODriverLog("stopping power up sequence ...");
		pmvAnd(pmv, ~PM_GNRIC_POWUP_SET);
		pmvOr(pmv, 0x30000); /* CFG */

		/*
		 * now do this all over again with the new CFG.
		 */
		beginPowerUpSequence(pmv);

		res = waitForPOWOK(pmv);
		if (res != kCprSuccess)
			return res;

		res = completePowerUpSequence(pmv);
		if (res != kCprSuccess)
			return res;

		BCM2708PowerDomain::stop();

		return kCprSuccess;
	}

	void resetPeripheralsUngated() {
		CM_PERIICTL = CM_PASSWORD | CM_PERIICTL_GATE_SET;

#ifdef __VIDEOCORE4__
		IODriverLog("calling v16mov gated ...");

		/*
		 * don't ask me.
		 */
		__asm volatile (
			"v16mov -, 0 REP32\n"
			"v16mov -, 0\n"
		);
#endif

		IODriverLog("ungating and resetting ...");

		CM_PERIICTL = CM_PASSWORD;

		waitOscTicks();

		*pmReg |= CM_PASSWORD | PM_IMAGE_PERIRSTN_SET;

		CM_PERIICTL = CM_PASSWORD | CM_PERIICTL_GATE_SET;

		IODriverLog("done");
	}

	virtual void start() override {
#if 1
		cpr_power_result_t res = powerOn();

		if (res != kCprSuccess)
			panic("%s failed to start the image power domain", driverName);

		resetPeripheralsUngated();
#else
		IODriverLog("image_domain_peripherals_init returned: 0x%X", image_domain_peripherals_init());
#endif

		IODriverLog("CM_PERIICTL = 0x%X", CM_PERIICTL);
		IODriverLog("CM_PERIIDIV = 0x%X", CM_PERIIDIV);
		IODriverLog("PM_IMAGE    = 0x%X", PM_IMAGE);

		IODriverLog("started");

		BCM2708PowerDomain::start();
	}
};

IODriverCreateSingletonInstance(BCM2708PowerDomainImage);

/***********************************************************************
 *
 * USB power domain.
 *
 ***********************************************************************/

struct BCM2708PowerDomainUSB : BCM2708PowerDomain {
	IODriverConstructor(BCM2708PowerDomainUSB);

	virtual void init() override {
		setName("BCM2708PowerDomainUSB");
		setPmReg(&PM_USB);
		registerPowerDomain(kCprPowerDomainUSB);
	}

	virtual void stop() override {
		IODriverLog("stopping ...");

		PM_USB = PM_PASSWORD;
		udelay(200000);

		IODriverLog("stopped");

		BCM2708PowerDomain::stop();
	}

	virtual void start() override {
		IODriverLog("starting ...");

		PM_USB |= PM_PASSWORD | PM_USB_CTRLEN_SET;
		udelay(600);

		IODriverLog("started");

		BCM2708PowerDomain::start();
	}
};

IODriverCreateSingletonInstance(BCM2708PowerDomainUSB);

/***********************************************************************
 *
 * ARM power domain.
 *
 ***********************************************************************/

struct BCM2708PowerDomainARM : BCM2708PowerDomain {
	IODriverConstructor(BCM2708PowerDomainARM);

	virtual void init() override {
		setName("BCM2708PowerDomainARM");
		setPmReg(&PM_PROC);
		rstnMask = PM_PROC_ARMRSTN_CLR;
		registerPowerDomain(kCprPowerDomainARM);
	}

	virtual void stop() override {
		panic("this power domain cannot be stopped");
	}

	virtual void start() override {
		IODriverLog("starting ...");

		cpr_power_result_t res = powerOn();

		if (res != kCprSuccess)
			panic("%s failed to start the ARM power domain", driverName);

		IODriverLog("started");

		BCM2708PowerDomain::start();
	}
};

IODriverCreateSingletonInstance(BCM2708PowerDomainARM);


// BCM2708Gpio.cc
//

#include <drivers/BCM2708Gpio.hpp>

void BCM2708Gpio::setFunction(uint8_t pin_num, BCM2708PinmuxSetting function) {
  volatile uint32_t* fsel = reinterpret_cast<volatile uint32_t*>(&GP_FSEL0);
  uint8_t regnum = pin_num / 10;

  uint8_t pin_shift = (pin_num % 10) * 3;

  fsel[regnum] = (fsel[regnum] & ~(0x7 << pin_shift)) | (function << pin_shift);
}

void BCM2708Gpio::getAllFunctions(BCM2708PinmuxSetting *functions) {
  volatile uint32_t* fsel = reinterpret_cast<volatile uint32_t*>(&GP_FSEL0);
  for (int bank=0; bank<7; bank++) {
    int pins_in_bank = (bank == 6) ? 4 : 10;
    for (int pin=0; pin < pins_in_bank; pin++) {
      functions[(bank * 10) + pin] = static_cast<BCM2708PinmuxSetting>((fsel[bank] >> (pin * 3)) & 0x7);
    }
  }
}

void BCM2708Gpio::clearPin(uint8_t pin_num) {
  volatile uint32_t *clear = reinterpret_cast<volatile uint32_t*>(&GP_CLR0);
  uint8_t regnum = pin_num / 32;
  uint8_t pin_shift = pin_num % 32;

  clear[regnum] = 1 << pin_shift;
}

void BCM2708Gpio::setPin(uint8_t pin_num) {
  volatile uint32_t *clear = reinterpret_cast<volatile uint32_t*>(&GP_SET0);
  uint8_t regnum = pin_num / 32;
  uint8_t pin_shift = pin_num % 32;

  clear[regnum] = 1 << pin_shift;
}

bool BCM2708Gpio::getPin(uint8_t pin_num) {
  const volatile uint32_t *level = reinterpret_cast<const volatile uint32_t*>(&GP_LEV0);
  uint8_t regnum = pin_num / 32;
  uint8_t pin_shift = pin_num % 32;

  return level[regnum] & (1 << pin_shift);
}

uint32_t BCM2708Gpio::getBank(uint8_t bank) {
  const volatile uint32_t *level = reinterpret_cast<const volatile uint32_t*>(&GP_LEV0);
  return level[bank];
}

BCM2708Gpio gGPIO {};

/*
 * VideoCore4_Drivers
 * Copyright (c) 2017 Kristina Brooks
 *
 * ARM control driver.
 */

#include <drivers/IODevice.hpp>
#include <drivers/BCM2708PowerManagement.hpp>
#include <drivers/BCM2708ClockDomains.hpp>
#include <drivers/BCM2708ArmControl.hh>
#include <string.h>
#include <stdio.h>
#include "utils.hh"
#include "otp.h"

#define FLAG_BUSY (1 << 31)

extern uint32_t g_CPUID;
extern uint32_t g_RAMSize;

extern uint8_t _binary_arm_chainloader_build_arm_chainloader_bin_start;
extern uint8_t _binary_arm_chainloader_build_arm_chainloader_bin_end;

#define ARM_MEMORY_BASE 0xC0000000
#define ARM_BKPT_OPCODE 0xE1200070

void bzero(void *addr, size_t len) {
  uint8_t *x = (uint8_t*)addr;
  for (size_t i = 0; i < len; i++) {
    x[i] = 0;
  };
}

static const uint8_t g_BrespTab[] = {
	0x10, 0x14, 0x10, 0x14, 0x10, 0x14, 0x10, 0x14, 0x10, 0x1C, 0x18, 0x1C, 0x18, 0x0,
	0x10, 0x14, 0x10, 0x1C, 0x10, 0x14, 0x10, 0x14, 0x10, 0x14, 0x10, 0x14, 0x10, 0x0,
	0x10, 0x14, 0x10, 0x1C, 0x18, 0x1C, 0x10, 0x14, 0x18, 0x1C, 0x10, 0x14, 0x10, 0x0,
	0x10, 0x14, 0x18, 0x14, 0x10, 0x14, 0x10, 0x14, 0x10, 0x14, 0x10, 0x14, 0x10, 0x0,
	0x10, 0x14, 0x18, 0x14, 0x18, 0x14, 0x10, 0x14, 0x10, 0x14, 0x10, 0x14, 0x18, 0x0
};

void BCM2708ArmControl::bridgeWriteBresp(uint8_t bits) {
  ARM_CONTROL0 = (ARM_CONTROL0 & ~(ARM_C0_BRESP1|ARM_C0_BRESP2)) | bits;
  udelay(30);
}

void BCM2708ArmControl::bridgeCycleBresp() {
  //IODriverLog("cycling through bresp bits ...");
  for (int i = 0; i < sizeof(g_BrespTab)/sizeof(g_BrespTab[0]); i++) {
    bridgeWriteBresp(g_BrespTab[i]);
  }
}

BCM2708ArmControl::BCM2708ArmControl() {
  _beforeInit();
  driverName = "BCM2708ArmControl";
  init();
  _afterInit();
}

void BCM2708ArmControl::init() {
  setTag('ARMC');
}

void BCM2708ArmControl::bridgeStart(bool cycleBrespBits) {
  //IODriverLog("setting up async bridge ...");

  if (cycleBrespBits) {
    pmDomain->setReset();
    bridgeCycleBresp();
    pmDomain->setReset();

    udelay(300);
  }

  IODriverLog("starting async bridge now!");
  ARM_CONTROL1 &= ~ARM_C1_REQSTOP;
  udelay(300);

  if (!cycleBrespBits) pmDomain->setReset();

  //IODriverLog("bridge init done, PM_PROC is now: 0x%X!", PM_PROC);
}

void BCM2708ArmControl::setupClock() {
  IODriverLog("initializing PLLB ...");

  /* oscillator->pllb */
  A2W_XOSC_CTRL |= A2W_PASSWORD | A2W_XOSC_CTRL_PLLBEN_SET;

  A2W_PLLB_FRAC = A2W_PASSWORD | 0xeaaa8; // out of 0x100000
  A2W_PLLB_CTRL = A2W_PASSWORD | 48 | 0x1000;

  CM_PLLB = CM_PASSWORD | CM_PLLB_DIGRST_SET | CM_PLLB_ANARST_SET;
  CM_PLLB = CM_PASSWORD | CM_PLLB_DIGRST_SET | CM_PLLB_ANARST_SET | CM_PLLB_HOLDARM_SET;

  A2W_PLLB_ANA3 = A2W_PASSWORD | 0x100;
  A2W_PLLB_ANA2 = A2W_PASSWORD | 0x0;
  A2W_PLLB_ANA1 = A2W_PASSWORD | 0x140000;
  A2W_PLLB_ANA0 = A2W_PASSWORD | 0x0;

  BCM2708VCO* pllb = static_cast<BCM2708VCO*>(IODevice::findByTag('PLLB'));
  assert(pllb);
  pllb->setDigValues();

  A2W_PLLB_ARM = A2W_PASSWORD | 2;

  CM_PLLB = CM_PASSWORD | CM_PLLB_DIGRST_SET | CM_PLLB_ANARST_SET | CM_PLLB_HOLDARM_SET | CM_PLLB_LOADARM_SET;
  CM_PLLB = CM_PASSWORD | CM_PLLB_DIGRST_SET | CM_PLLB_ANARST_SET | CM_PLLB_HOLDARM_SET;
  CM_PLLB = CM_PASSWORD;

  CM_ARMCTL = CM_PASSWORD | 4 | CM_ARMCTL_ENAB_SET;

  IODriverLog("KAIP  = 0x%X", A2W_PLLB_ANA_KAIP); /* 0x228 */
  IODriverLog("MULTI = 0x%X", A2W_PLLB_ANA_MULTI); /* 0x613277 */

  IODriverLog("ARM clock succesfully initialized!");
}


void BCM2708ArmControl::setupOtherClocks() {
  // `compatible = "brcm,bcm2835-cprman";` in DT expects PLLA and PLLD to be functioning
  // enable them here
  A2W_XOSC_CTRL |= A2W_PASSWORD | A2W_XOSC_CTRL_PLLAEN_SET;
  A2W_XOSC_CTRL |= A2W_PASSWORD | A2W_XOSC_CTRL_PLLDEN_SET;
}

void BCM2708ArmControl::patchFirmwareData(struct OtpInfo *info) {
  volatile firmware_arm_data_t* firmware_data = reinterpret_cast<firmware_arm_data_t*>(ARM_MEMORY_BASE + 32);

  firmware_data->sdram_size = g_RAMSize;
  firmware_data->vpu_cpuid = g_CPUID;
  firmware_data->serial = info->serial;
  firmware_data->revision = info->revision;
}

void BCM2708ArmControl::loadInitialCode(struct OtpInfo *info) {
  uint32_t* mem = (uint32_t*)(ARM_MEMORY_BASE);

  uint8_t* start = &_binary_arm_chainloader_build_arm_chainloader_bin_start;
  uint8_t* end = &_binary_arm_chainloader_build_arm_chainloader_bin_end;
  uint32_t size = (uint32_t)(end - start);

  memcpy(mem, start, size);

  IODriverLog("copied %d bytes to 0x%X!", size, ARM_MEMORY_BASE);

  /* verify */
  for (int i = 0; i < size; i++) {
    uint8_t* mem8 = (uint8_t*)(mem);
    if (start[i] != mem8[i])
      panic("copy failed at 0x%lX expected 0x%lX, got 0x%lX", (uint32_t)&mem8[i],
            *((uint32_t*)&mem8[i]),
            *((uint32_t*)&start[i]));
  }

  patchFirmwareData(info);
}

// maps a 16mb chunk of ram
// the bus address has a resolution of 2mb
// the arm addr has a resolution of 16mb
// the entire mapping is 16mb long
// comments say there are 32 slots in the list (512mb mapped) an another 32 spare (1gig mapped)
void BCM2708ArmControl::mapBusToArm(uint32_t busAddr, uint32_t armAddr) {
  volatile uint32_t* tte = reinterpret_cast<volatile uint32_t*>(&ARM_TRANSLATE);

  uint32_t index = armAddr >> 24; // div by 16mb
  uint32_t pte = busAddr >> 21; // div by 2mb
  //IODriverLog("mapBusToArm index:%x, pte:%x", index, pte);

  tte[index] = pte;
}

void BCM2708ArmControl::printregs() {
  uint32_t c0 = ARM_CONTROL0, c1 = ARM_CONTROL1;
  IODriverLog("C0: %x C1: %x", c0, c1);
}

void BCM2708ArmControl::bzero2(void *addr, size_t len) {
  uint8_t *x = (uint8_t*)addr;
  for (size_t i = 0; i < len; i++) {
    if ((i % 0x1000000) == 0) IODriverLog("%x", i);
    x[i] = 0;
  };
}

void BCM2708ArmControl::start(struct OtpInfo *info) {
  IODriverLog("arm starting ...");

  pmDomain = PowerManagementDomain::getDeviceForDomain(kCprPowerDomainARM);
  assert(pmDomain);

  //IODriverLog("about to zero out, stack near %x", pmDomain);
  //bzero2((void*)ARM_MEMORY_BASE, 1024*1024*512);
  //IODriverLog("zeroed");
  loadInitialCode(info);
  //uint8_t aarch64_loop[] = { 0x00, 0x00, 0x00, 0x14 };
  //for (int i=0; i<0x300; i += 4) {
    //memcpy((void*)(ARM_MEMORY_BASE + i), aarch64_loop, 4);
  //}

  IODriverLog("original memstart: 0x%X", *((volatile uint32_t*)ARM_MEMORY_BASE));

  // map 62 chunks of 16mb of ram, total of 992mb
  for (uint32_t i = 0; i < 62; i++) {
          uint32_t offset = i * 0x1000000;
          mapBusToArm(ARM_MEMORY_BASE + offset, 0x0 + offset);
          //IODriverLog("mapped VC 0x%X to ARM 0x%X", ARM_MEMORY_BASE + offset, 0x0 + offset);
  }

  //IODriverLog("mapped VC 0x%X to ARM 0x%X", ARM_MEMORY_BASE, 0);

  mapBusToArm(VC4_PERIPH_BASE, ARM_PERIPH_BASE);

  IODriverLog("mapped peripherals VC 0x%X to ARM 0x%X", VC4_PERIPH_BASE, ARM_PERIPH_BASE);

  /* see if the ARM block is responding */
  IODriverLog("ARM ID: 0x%X C0: 0x%X", ARM_ID, ARM_CONTROL0);

#if 0
  enable_jtag();
  ARM_CONTROL0 |= ARM_C0_JTAGGPIO;
#endif
  bool levels[64];
  enum BCM2708PinmuxSetting functions[64];
  gpio_snapshot(levels, functions);
  gpio_print_snapshot(levels, functions);

  /*
   * enable peripheral access, map arm secure bits to axi secure bits 1:1 and
   * set the mem size for who knows what reason.
   */
  ARM_CONTROL0 |= 0x008
                | ARM_C0_APROTPASS | ARM_C0_APROTMSK  // allow both kernel and userland to access mmio
                | ARM_C0_SIZ1G
                | ARM_C0_FULLPERI;                    // allow access to all peripherals
  // | ARM_C0_AARCH64;
  ARM_CONTROL1 |= ARM_C1_PERSON;

  ARM_IRQ_ENBL3 |= ARM_IE_MAIL;

  printregs();

  setupClock();
  //setupOtherClocks();
  pmDomain->start();
  printregs();
  IODriverLog("arm is on, starting async bridge");

  /*
   * ARM is now powered on but stalling on a bus transaction, start the
   * async bridge and let ARM start fetching instructions.
   */
  bridgeStart(true);

  IODevice::start();
}

