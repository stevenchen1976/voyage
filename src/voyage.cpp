/*
  Voyage A library for creating and managing hooks for unix systems authored by Gabriel.
  <https://github.com/0x41337/voyage>     <041337@proton.me>

  Voyage is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Voyage is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <voyage.h>
#include <trampoline.hpp>

#include <dlfcn.h>
#include <memory.h>

/// @brief Search by symbol address
/// @param img_name Image/module name
/// @param sym_name Symbol name
/// @param dest Destination that will receive the found address
/// @return `VE_STATUS` can be [`VE_ERROR_MODULE_NOT_FOUND`, `VE_ERROR_SYMBOL_NOT_FOUND` and `VE_OK`]
VE_STATUS VE_FindSymbolAddress(const char *img_name, const char *sym_name, void *dest)
{
    // Find the image
    void *handle = dlopen(img_name, RTLD_NOW | RTLD_GLOBAL);
    if (handle == nullptr)
    {
        return VE_ERROR_MODULE_NOT_FOUND;
    }

    // Find symbol in the image
    void *symbol = dlsym(handle, sym_name);
    if (symbol == nullptr)
    {
        return VE_ERROR_SYMBOL_NOT_FOUND;
    }

    // Copy address of symbol to destination
    memcpy(dest, &symbol, sizeof(void *));

    // Close module
    dlclose(handle);

    return VE_OK;
}

/// @brief Create a hook
/// @param target_address Address to the target
/// @param hook_address Address to the hook
/// @return `VE_STATUS` can be [`VE_ERROR_ALREADY_CREATED`, `VE_ERROR_NOT_CREATED`, `VE_ERROR_MEMORY_PROTECT` and `VE_OK`]
VE_STATUS VE_CreateHook(void *target_address, void *hook_address, Hook *dest)
{
    // TODO: Check if there is already a hook for this address

    // Ensures that the memory page is writable and executable
    auto status = Remove_memory_protection(target_address);
    if (status != VE_OK)
        return status;

    // Save the original instructions
    Save_original_instructions(target_address, dest);

    // Apply JMP injection to the target address
    Apply_detour(target_address, hook_address);

    // Restores memory protection
    status = Restore_memory_protection(target_address);
    if (status != VE_OK)
        return status;

    // Configure the hook
    dest->target = target_address;
    dest->enabled = true;

    return VE_OK;
}