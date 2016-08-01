/**
 * @file
 */

#pragma once

#include "ui/Window.h"
#include "core/Common.h"
#include "core/EMailValidator.h"
#include "../Client.h"

namespace frontend {

class SignupWindow: public ui::Window {
private:
	Client* _client;
public:
	SignupWindow(Client* client) :
			ui::Window(client), _client(client) {
		core_assert_always(loadResourceFile("ui/window/signup.tb.txt"));
		SetSettings(tb::WINDOW_SETTINGS_TITLEBAR);
	}

	bool OnEvent(const tb::TBWidgetEvent &ev) override {
		if (ev.type != tb::EVENT_TYPE_CLICK) {
			return ui::Window::OnEvent(ev);
		}
		if (ev.target->GetID() == TBIDC("signup")) {
			const std::string& email = getStr("email");
			const std::string& password = getStr("password");
			const std::string& passwordVerify = getStr("password_verify");
			if (password != passwordVerify) {
				popup(_("error"), _("passwordsdonotmatch"));
				return true;
			}
			if (!core::isValidEmail(email)) {
				popup(_("error"), _("emailinvalid"));
				return true;
			}
			_client->signup(email, password);
			return true;
		} else if (ev.target->GetID() == TBIDC("cancel")) {
			Close();
			return true;
		}
		return ui::Window::OnEvent(ev);
	}
};

}