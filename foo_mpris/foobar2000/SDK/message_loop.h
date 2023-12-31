#pragma once

#ifdef _WIN32
class NOVTABLE message_filter
{
public:
    virtual bool pretranslate_message(MSG * p_msg) = 0;
};
 
class NOVTABLE idle_handler {
public:
    virtual bool on_idle() = 0;
};

class NOVTABLE message_loop : public service_base
{
public:
    virtual void add_message_filter(message_filter * ptr) = 0;
    virtual void remove_message_filter(message_filter * ptr) = 0;
 
    virtual void add_idle_handler(idle_handler * ptr) = 0;
    virtual void remove_idle_handler(idle_handler * ptr) = 0;

	FB2K_MAKE_SERVICE_COREAPI(message_loop);
};

class NOVTABLE message_loop_v2 : public message_loop {
public:
	virtual void add_message_filter_ex(message_filter * ptr, t_uint32 lowest, t_uint32 highest) = 0;

	FB2K_MAKE_SERVICE_COREAPI_EXTENSION(message_loop_v2, message_loop);
};

class message_filter_impl_base : public message_filter {
public:
	message_filter_impl_base() {message_loop::get()->add_message_filter(this);}
	message_filter_impl_base(t_uint32 lowest, t_uint32 highest) {
		message_loop_v2::get()->add_message_filter_ex(this, lowest, highest);
	}
	~message_filter_impl_base() {message_loop::get()->remove_message_filter(this);}
	bool pretranslate_message(MSG * p_msg) {return false;}
	
	PFC_CLASS_NOT_COPYABLE(message_filter_impl_base,message_filter_impl_base);
};

class message_filter_impl_accel : public message_filter_impl_base {
protected:
	bool pretranslate_message(MSG * p_msg) {
		if (m_wnd != NULL) {
			if (GetActiveWindow() == m_wnd) {
				if (TranslateAccelerator(m_wnd,m_accel.get(),p_msg) != 0) {
					return true;
				}
			}
		}
		return false;
	}
public:
	message_filter_impl_accel(HINSTANCE p_instance,const TCHAR * p_accel) : m_wnd(NULL) {
		m_accel.load(p_instance,p_accel);
	}
	void set_wnd(HWND p_wnd) {m_wnd = p_wnd;}
private:
	HWND m_wnd;
	win32_accelerator m_accel;

	PFC_CLASS_NOT_COPYABLE(message_filter_impl_accel,message_filter_impl_accel);
};

class message_filter_remap_f1 : public message_filter_impl_base {
public:
	bool pretranslate_message(MSG * p_msg) {
		if (IsOurMsg(p_msg) && m_wnd != NULL && GetActiveWindow() == m_wnd) {
			::PostMessage(m_wnd, WM_SYSCOMMAND, SC_CONTEXTHELP, -1);
			return true;
		}
		return false;
	}
	void set_wnd(HWND wnd) {m_wnd = wnd;}
private:
	static bool IsOurMsg(const MSG * msg) {
		return msg->message == WM_KEYDOWN && msg->wParam == VK_F1;
	}
	HWND m_wnd;
};

class idle_handler_impl_base : public idle_handler {
public:
	idle_handler_impl_base() {message_loop::get()->add_idle_handler(this);}
	~idle_handler_impl_base() { message_loop::get()->remove_idle_handler(this);}
	bool on_idle() {return true;}

	PFC_CLASS_NOT_COPYABLE_EX(idle_handler_impl_base)
};
#endif // _WIN32
