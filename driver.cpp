#include <dlfcn.h>

#include "driver.h"

#include "zlog.h"

#define LOAD_SYM_OR_RETURN(sym_name) do { \
    driver->sym_name = (typeof(driver->sym_name))dlsym(lib, #sym_name); \
    if(!driver->sym_name){ \
        fprintf(stderr, "Could not load symbol %s in driver %s\n", #sym_name, name.c_str()); \
        return nullptr; \
    } \
} while(0)

namespace mho {

std::vector<std::string> get_availible_drivers(){
    // FIXME:
    // open dir listing, etc...

    std::vector<std::string> list;
    list.push_back("faux-driver");

    return list;
}

driver_t *load_driver(std::string name, MhoDB *db){
    std::string libname = "drivers/lib" + name + ".so";
    LOG(name << " " << libname);

    void *lib = dlopen(libname.c_str(), RTLD_LAZY);
    if(!lib){
        // FIXME: log error
        // FIXME: somehow report back to UI more than just "failure..."

        perror("failed to open driver");
        return nullptr;
    }

    driver_t *driver = new driver_t();
    if(!driver){
        // FIXME: log error
        // FIXME: somehow report back to UI more than just "failure..."

        perror("failed to alloc mem");
        dlclose(lib);
        return nullptr;
    }

    driver->lib = lib;

    LOAD_SYM_OR_RETURN(name);
    LOAD_SYM_OR_RETURN(load);
    LOAD_SYM_OR_RETURN(unload);
    LOAD_SYM_OR_RETURN(error);
    LOAD_SYM_OR_RETURN(add_to_util_list);
    LOAD_SYM_OR_RETURN(discover_device_addresses);
    LOAD_SYM_OR_RETURN(connect_to_device);
    LOAD_SYM_OR_RETURN(disconnect_device);
    LOAD_SYM_OR_RETURN(disconnect_all_devices);
    LOAD_SYM_OR_RETURN(get_device_address);
    LOAD_SYM_OR_RETURN(do_poll);

    driver->load(db);

    LOG("Loaded driver \"" << name << "\" = " << driver);

    return driver;
}

bool unload_driver(driver_t *driver){
    driver->disconnect_all_devices();
    driver->unload();

    LOG("Unloading driver \"" << driver->name() << "\" = " << driver);

    dlclose(driver->lib);

    delete driver;

    return true;
}

} // mho

