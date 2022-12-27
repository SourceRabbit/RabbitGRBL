#include "../Grbl.h"

namespace WebUI {

    void remove_password(char* str, AuthenticationLevel& auth_level) { auth_level = AuthenticationLevel::LEVEL_ADMIN; }

}
