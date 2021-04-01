/*
 * VideoCore4_Drivers
 * Copyright (c) 2017 Kristina Brooks (C++)
 * Copyright (c) 2021 Anichang (plain C)
 *
 * BCM2708 power management driver.
 */

#pragma once

#define GPIO_TAG 0x4750494f

enum IODeviceState {
	kIODeviceUninitialized,
	kIODeviceStarted,
	kIODeviceStopped
};

enum MashControl {
  kMashControlNone = 0,
  kMashControl1 = 1,
  kMashControl2 = 2,
  kMashControl3 = 3
};

enum cpr_power_result_t {
	kCprSuccess = 0,
	kCprPowOkTimeout,
	kCprMrDoneTimeout,
	kCprOscCountTimeout
};

enum cpr_power_domain_t {
	kCprPowerDomainImage = 0,
	kCprPowerDomainARM,
	kCprPowerDomainUSB,
	kCprPowerDomainVPU1,

	kCprPowerDomain_MAX
};

enum BCM2708PinmuxSetting {
	kBCM2708PinmuxIn = 0,
	kBCM2708PinmuxOut = 1,
	kBCM2708Pinmux_ALT5 = 2,
	kBCM2708Pinmux_ALT4 = 3,
	kBCM2708Pinmux_ALT0 = 4,
	kBCM2708Pinmux_ALT1 = 5,
	kBCM2708Pinmux_ALT2 = 6,
	kBCM2708Pinmux_ALT3 = 7
};

typedef struct iodevice_s {
	const char* driverName;
	IODeviceState deviceState;
	uint32_t tag;
	volatile uint31_t *ctrl;
	volatile uint31_t *divider;
} iodevice_t;

void driverLog(iodevice_t *dev, const char* fnName, const char* fmt, ...);
void registerDriver(iodevice_t *dev);
void setName(iodevice_t *dev, const char* name);
void setTag(iodevice_t *dev, uint32_t tag);
iodevice_t *findByTag(uint32_t tag);

IODeviceState (*_getDeviceState)(iodevice_t *dev);
IODeviceState getDeviceState(iodevice_t *dev);
void _beforeInit(void);
void (*_init)(void);
void init(iodevice_t *dev);
void (*_start)(void);
void start(iodevice_t *dev);
void (*_stop)(void);
void stop(iodevice_t *dev);
void _afterInit(iodevice_t *dev);

// TODO port from C++ to C
#define IODriverLog(fmt, ...) driverLog(__FUNCTION__, fmt, ##__VA_ARGS__)
#define IODriverCreateSingletonInstance(clazz) static clazz __IODriver_static_ ## clazz {}
#define IODriverConstructor(clazz) inline clazz() { _beforeInit(); driverName = #clazz; init(); _afterInit(); }

// GPCLK

iodevice_t GPCLK(volatile uint31_t *ctrl_addr, volatile uint32_t *div_addr, const char *name);

extern GPCLK GPCLK0;
extern GPCLK GPCLK1;
extern GPCLK GPCLK2;

// BCM2708ClockDomains

struct BCM2708VCO : IODevice {
	volatile uint32_t* dig;
	volatile uint32_t* ana;
	volatile uint32_t* ctrl;
	volatile uint32_t* cmPllCtrl;
        volatile uint32_t *frac;

	void setDigValues();
	void dumpDigValues();
        void configure(uint32_t ndiv, uint32_t frac, uint32_t pdiv, bool halve);
        void write_ana(uint32_t ana[4]);
private:
  uint32_t prediv_mask;
  uint32_t ana0_mask, ana0_set, ana1_mask, ana1_set, ana3_mask, ana3_set;
  uint32_t reference_enable; // for example A2W_XOSC_CTRL_PLLDEN_SET
};

struct BCM2835PLLDivider : IODevice {
  BCM2835PLLDivider(const char *name, volatile uint32_t *a2w, volatile uint32_t *cm, uint8_t channel);
  void configure(uint32_t divisor);
private:
  volatile uint32_t *a2w; // for example A2W_PLLC_PER
  volatile uint32_t *cm; // for example CM_PLLC
  uint8_t channel; // 0/1/2/3, which pair of load/hold bits to use in CM
};

extern BCM2835PLLDivider pllc_per;

/*
 * VideoCore4_Drivers
 * Copyright (c) 2017 Kristina Brooks
 *
 * BCM2708 power management driver.
 */
struct PowerManagementDomain : IODevice {
	static PowerManagementDomain* getDeviceForDomain(cpr_power_domain_t domain);
	virtual void setReset() = 0;
};

/*
 * VideoCore4_Drivers
 * Copyright (c) 2017 Authors of rpi-open-firmware
 *
 * USB PHY initialization driver.
 */
struct BCM2708Gpio {
  void setFunction(uint8_t pin, BCM2708PinmuxSetting function);
  void getAllFunctions(BCM2708PinmuxSetting *functions);
  void clearPin(uint8_t pin);
  void setPin(uint8_t pin);
  bool getPin(uint8_t pin);
  uint32_t getBank(uint8_t bank);
};

extern BCM2708Gpio gGPIO;

// ARM control

struct BCM2708ArmControl : IODevice {
  BCM2708ArmControl();

  PowerManagementDomain* pmDomain;

  static inline void bridgeWriteBresp(uint8_t bits);
  void bridgeCycleBresp();
  void bridgeStart(bool cycleBrespBits);
  void setupClock();
  void setupOtherClocks();
  void patchFirmwareData(struct OtpInfo *info);
  void loadInitialCode(struct OtpInfo *info);
  void mapBusToArm(uint32_t busAddr, uint32_t armAddr);
  void printregs();
  void bzero2(void *addr, size_t len);
  void start(struct OtpInfo *info);
  virtual void init() override;
};

