#pragma once

#include <cstdint>

#include <mpi/core/exception.hpp>
#include <mpi/core/mpi.hpp>
#include <mpi/tool/structs/control_variable.hpp>

namespace mpi::tool
{
template <typename type>
class control_variable_handle
{
public:
  explicit control_variable_handle  (const control_variable& variable, type& data)
  : managed_(true)
  {
    MPI_CHECK_ERROR_CODE(MPI_T_cvar_handle_alloc, (variable.index, static_cast<void*>(&data), &native_, &count_))
  }
  explicit control_variable_handle  (const MPI_T_cvar_handle& native)
  : native_(native)
  {

  }
  control_variable_handle           (const control_variable_handle&  that) = delete;
  control_variable_handle           (      control_variable_handle&& temp) noexcept
  : managed_(temp.managed_), native_(temp.native_), count_(temp.count_)
  {
    temp.managed_ = false;
    temp.native_  = MPI_T_CVAR_HANDLE_NULL;
    temp.count_   = 0;
  }
  virtual ~control_variable_handle  ()
  {
    if (managed_ && native_ != MPI_T_CVAR_HANDLE_NULL)
      MPI_CHECK_ERROR_CODE(MPI_T_cvar_handle_free, (&native_))
  }
  control_variable_handle& operator=(const control_variable_handle&  that) = delete;
  control_variable_handle& operator=(      control_variable_handle&& temp) noexcept
  {
    if (this != &temp)
    {
      if (managed_ && native_ != MPI_T_CVAR_HANDLE_NULL)
        MPI_CHECK_ERROR_CODE(MPI_T_cvar_handle_free, (&native_))

      managed_      = temp.managed_;
      native_       = temp.native_ ;
      count_        = temp.count_  ; 

      temp.managed_ = false;
      temp.native_  = MPI_T_CVAR_HANDLE_NULL;
      temp.count_   = 0;
    }
    return *this;
  }

  type read () const
  {
    type result;
    MPI_CHECK_ERROR_CODE(MPI_T_cvar_read , (native_, static_cast<void*>(&result)))
    return result;
  }
  void write(const type& value) const
  {
    MPI_CHECK_ERROR_CODE(MPI_T_cvar_write, (native_, static_cast<const void*>(&value)))
  }

protected:
  bool              managed_ = false;
  MPI_T_cvar_handle native_  = MPI_T_CVAR_HANDLE_NULL;
  std::int32_t      count_   = 0;
};
}