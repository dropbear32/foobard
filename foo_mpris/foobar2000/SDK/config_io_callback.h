#pragma once

//! Implementing this interface lets you maintain your own configuration files rather than depending on the cfg_var system. \n
//! Note that you must not make assumptions about what happens first: config_io_callback::on_read(), initialization of cfg_var values or config_io_callback::on_read() in other components. Order of these things is undefined and will change with each run. \n
//! Use service_factory_single_t<myclass> to register your implementations. Do not call other people's implementations, core is responsible for doing that when appropriate.
class NOVTABLE config_io_callback : public service_base {
public:
	//! Called on startup. You can read your configuration file from here. \n
	//! Hint: use core_api::get_profile_path() to retrieve the path of the folder where foobar2000 configuration files are stored.
	virtual void on_read() = 0;
	//! Called typically on shutdown but you should expect a call at any point after on_read(). You can write your configuration file from here.
	//! Hint: use core_api::get_profile_path() to retrieve the path of the folder where foobar2000 configuration files are stored.
	//! @param reset If set to true, our configuration is being reset, so you should wipe your files rather than rewrite them with current configuration.
	//! Since foobar2000 v2.0, reset is never issued - profile folder is cleared instead.
	virtual void on_write(bool reset) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(config_io_callback);
};

//! \since 1.0
class NOVTABLE config_io_callback_v2 : public config_io_callback {
	FB2K_MAKE_SERVICE_INTERFACE(config_io_callback_v2, config_io_callback)
public:
	//! Implement optionally. Called to quickly flush recent configuration changes. If your instance of config_io_callback needs to perform timeconsuming tasks when saving, you should skip implementing this method entirely.
	virtual void on_quicksave() = 0;
};

//! \since 1.4
//! New methods take a filesystem object that should be used for the update, so the whole config update can be performed as one transacted filesystem operation. \n
//! The core performs necessary checks to ensure that the volume where our profile resides is supports transacted operations. \n
//! However there are odd cases of people junctioning the profile folder and such. We cannot guarantee that your code won't run into such cases. \n
//! If you get a exception_io_transactions_unsupported, let the caller deal with it - your call will be retried with a regular filesystem instead of a transacted one.
class NOVTABLE config_io_callback_v3 : public config_io_callback_v2 {
	FB2K_MAKE_SERVICE_INTERFACE(config_io_callback_v3, config_io_callback_v2);
public:
	void on_quicksave();
	void on_write(bool bReset);
	//! Since foobar2000 v2.0, reset is never issued - profile folder is cleared instead.
	virtual void on_reset_v3( filesystem::ptr fs ) = 0;
	virtual void on_write_v3( filesystem::ptr fs ) = 0;
	virtual void on_quicksave_v3( filesystem::ptr fs ) = 0;
};

// For internal use in fb2k core
#define FB2K_PROFILE_CONFIG_READS_WRITES 0