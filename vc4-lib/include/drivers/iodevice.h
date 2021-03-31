#pragma once

enum IODeviceState {
	kIODeviceUninitialized,
	kIODeviceStarted,
	kIODeviceStopped
};

typedef struct iodevice_s {
	const char* driverName;
	IODeviceState deviceState;
	uint32_t tag;
} iodevice_t;

void driverLog(iodevice_t *dev, const char* fnName, const char* fmt, ...);
void registerDriver(iodevice_t *dev);
void setName(const char* name);
void setTag(uint32_t tag);
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

