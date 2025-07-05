// Copyright 2018 The Ripple Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "plugin0.h"

#include <cstdlib>

namespace mysql_ripple {

    namespace plugin {

        static Plugin DummyEndOfList = { nullptr };

        extern Plugin plugin_mysql_server_port_tcpip, DummyEndOfList;

        static Plugin *plugins[] = {
            &plugin_mysql_server_port_tcpip,
            &DummyEndOfList
          };

        bool InitPlugins() {
            for (Plugin *p : plugins) {
                if (p == nullptr || p->Init == nullptr)
                    continue;
                if (p->Init() != true)
                    return false;
            }
            return true;
        }

    }  // namespace plugin

}  // namespace mysql_ripple