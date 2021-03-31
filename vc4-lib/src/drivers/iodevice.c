
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

iodevice_t *findByTag(uint32_t tag) {
	for (unsigned int i = 0; i < (sizeof(g_DeviceRegistry) / sizeof(void*)); i++) {
		iodevice_t *dev = g_DeviceRegistry[i];
		if (dev && dev->tag == tag) {
			return dev;
		}
	}

	return nullptr;
}

