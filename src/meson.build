conf_data = configuration_data()
conf_data.set('TAG', 'libusrio_gpio_controller')
conf_data.set('DEBUG_OPT', 'CONFIG_LIBUSRIO_DEBUG_GPIO')
conf_data.set('PREFIX', 'gpio_controller')
conf_data.set('FUNC', '_log_var->_data->log_cb')

configure_file(input : log_macros_tmpl,
               output : 'gpio_controller_log.h',
               configuration : conf_data)

conf_data = configuration_data()
conf_data.set('TAG', 'libusrio_i2c_controller')
conf_data.set('DEBUG_OPT', 'CONFIG_LIBUSRIO_DEBUG_I2C')
conf_data.set('PREFIX', 'i2c_controller')
conf_data.set('FUNC', '(_log_var)->log_cb')

configure_file(input : log_macros_tmpl,
               output : 'i2c_controller_log.h',
               configuration : conf_data)

conf_data = configuration_data()
conf_data.set('TAG', 'libusrio_spi_controller')
conf_data.set('DEBUG_OPT', 'CONFIG_LIBUSRIO_DEBUG_SPI')
conf_data.set('PREFIX', 'spi_controller')
conf_data.set('FUNC', '_log_var->_data->log_cb')

configure_file(input : log_macros_tmpl,
               output : 'spi_controller_log.h',
               configuration : conf_data)

libusrio_sources = [
    'gpio_controller.c',
    'i2c_controller.c',
    'spi_controller.c',
    'mfd.c'
]

deps = [  libdpgc_dep ]

# For gpiod
if get_option('libusrio_gpiod')
	libgpiod_dep = dependency('libgpiod', required : true)
	deps += [ libgpiod_dep ]
	libusrio_sources += [ 'gpiod.c' ]
endif

# For i2cdev
if get_option('libusrio_i2cdev')
	libusrio_sources += [ 'i2cdev.c' ]
endif

# For spidev
if get_option('libusrio_spidev')
	libusrio_sources += [ 'spidev.c' ]
endif

# For dummy
if get_option('libusrio_dummy')
	libusrio_sources += [ 'dummy.c' ]
endif

libusrio = shared_library('usrio',
                           libusrio_sources,
                           include_directories: inc,
                           install : true,
                           dependencies : deps)
