#ifndef slic3r_PrintHostSendDialog_perl_hpp_
#define slic3r_PrintHostSendDialog_perl_hpp_

#include <string>
#include <memory>


// Perl headers collide with some of the wx headers,
// which is why this wrapper eixsts.
// TODO: remove this once the binding is not needed.


namespace Slic3r {

class PrintHostSendDialog;

class PrintHostSendDialog_perl
{
private:
	std::unique_ptr<PrintHostSendDialog> dialog;

public:
	PrintHostSendDialog_perl(std::string path, bool can_start_print, bool can_simulate_print);
	~PrintHostSendDialog_perl();

	int ShowModal();

	std::string remote_path() const;                // returns path in utf-8
	bool print() const;                             // whether printing should be started
	bool simulate() const;							// whether simulation should be started
};

}

#endif
