#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include <mpi/core/type/data_type.hpp>
#include <mpi/core/utility/container_adapter.hpp>
#include <mpi/core/exception.hpp>
#include <mpi/core/mpi.hpp>

namespace mpi::io
{
class data_representation
{
public:
  using conversion_function_type = std::int32_t (*) (void*, MPI_Datatype, std::int32_t, void*, MPI_Offset, void*);
  using extent_function_type     = std::int32_t (*) (       MPI_Datatype, MPI_Aint*                      , void*);

  data_representation           (
    const std::string&             name           ,
    const conversion_function_type read_function  ,
    const conversion_function_type write_function ,
    const extent_function_type     extent_function,
    void*                          user_data      )
  : name_(name)
  {
    MPI_CHECK_ERROR_CODE(MPI_Register_datarep, (name.c_str(), read_function, write_function, extent_function, user_data))
  }
  data_representation           (
    const std::string&                                                                           name           ,
    const std::function<std::int32_t(void*, const data_type&, std::int32_t, void*, MPI_Offset)>& read_function  ,
    const std::function<std::int32_t(void*, const data_type&, std::int32_t, void*, MPI_Offset)>& write_function ,
    const std::function<std::int32_t(       const data_type&, MPI_Aint*                      )>& extent_function)
  : name_(name), read_function_(read_function), write_function_(write_function), extent_function_(extent_function)
  {
    MPI_CHECK_ERROR_CODE(MPI_Register_datarep, (
      name.c_str(), 
      [ ] (void* buffer, MPI_Datatype data_type, std::int32_t count  , void* file_buffer, MPI_Offset position, void* extra_state)
      {
        return static_cast<data_representation*>(extra_state)->read_function_ (buffer, mpi::data_type(data_type), count, file_buffer, position);
      }, 
      [ ] (void* buffer, MPI_Datatype data_type, std::int32_t count  , void* file_buffer, MPI_Offset position, void* extra_state)
      {
        return static_cast<data_representation*>(extra_state)->write_function_(buffer, mpi::data_type(data_type), count, file_buffer, position);
      }, 
      [ ] (              MPI_Datatype data_type, MPI_Aint*    extent                                               , void* extra_state)
      {
        return static_cast<data_representation*>(extra_state)->extent_function_(       mpi::data_type(data_type), extent);
      },
      this))
  }
  explicit data_representation  (const std::string& name = "external32")
  : name_(name)
  {

  }
  data_representation           (const data_representation&  that) = default;
  data_representation           (      data_representation&& temp) = default;
  virtual ~data_representation  ()                                 = default;
  data_representation& operator=(const data_representation&  that) = default;
  data_representation& operator=(      data_representation&& temp) = default;

  [[nodiscard]]
  MPI_Aint           pack_size(const std::int32_t size, const data_type& data_type) const
  {
    MPI_Aint result;
    MPI_CHECK_ERROR_CODE(MPI_Pack_external_size, (name_.c_str(), size, data_type.native(), &result))
    return result;
  }
  [[nodiscard]]
  MPI_Aint           pack     (const void* input , const std::int32_t input_size , const data_type&   input_data_type ,
                                     void* output, const MPI_Aint     output_size, const MPI_Aint     output_position ) const
  {
    MPI_Aint result(output_position);
    MPI_CHECK_ERROR_CODE(MPI_Pack_external     , (name_.c_str(), input, input_size, input_data_type.native(), output, output_size, &result ))
    return result;
  }
  template <typename input_type, typename output_type> [[nodiscard]]
  MPI_Aint           pack     (const input_type& input, const output_type& output, const MPI_Aint     output_position ) const
  {
    using input_adapter  = container_adapter<input_type >;
    using output_adapter = container_adapter<output_type>;

    return pack(input_adapter::data(input), static_cast<std::int32_t>(input_adapter::size(input)), input_adapter::data_type(), output_adapter::data(output), static_cast<MPI_Aint>(output_adapter::size(output)), output_position);
  }
  [[nodiscard]]
  MPI_Aint           unpack   (const void* input , const MPI_Aint     input_size , const MPI_Aint     input_position  ,
                                     void* output, const std::int32_t output_size, const data_type&   output_data_type) const
  {
    MPI_Aint result(input_position);
    MPI_CHECK_ERROR_CODE(MPI_Unpack_external   , (name_.c_str(), input, input_size, &result, output, output_size, output_data_type.native()))
    return result;
  }
  template <typename input_type, typename output_type> [[nodiscard]]
    MPI_Aint         unpack   (const input_type& input, const output_type& output, const MPI_Aint     input_position  ) const
  {
    using input_adapter  = container_adapter<input_type >;
    using output_adapter = container_adapter<output_type>;

    return unpack(input_adapter::data(input), static_cast<MPI_Aint>(input_adapter::size(input)), input_position, output_adapter::data(output), static_cast<std::int32_t>(output_adapter::size(output)), output_adapter::data_type());
  }

  [[nodiscard]]
  const std::string& name     () const
  {
    return name_;
  }
  
protected:
  std::string                                                                           name_           ;
  std::function<std::int32_t(void*, const data_type&, std::int32_t, void*, MPI_Offset)> read_function_  ;
  std::function<std::int32_t(void*, const data_type&, std::int32_t, void*, MPI_Offset)> write_function_ ;
  std::function<std::int32_t(       const data_type&, MPI_Aint*                      )> extent_function_;

};
}