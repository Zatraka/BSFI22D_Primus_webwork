#ifndef COUNTDTO_HPP
#define COUNTDTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)
<<<<<<< HEAD

class CountDto : public oatpp::DTO {

    DTO_INIT(CountDto, DTO);

    DTO_FIELD(oatpp::UInt32, count);
=======
class CountDto : public oatpp::DTO {

    DTO_INIT(CountDto, DTO)

        DTO_FIELD(oatpp::Int32, count);
>>>>>>> db1965b318089d88d214892967e8600fa806f9c6

};

#include OATPP_CODEGEN_END(DTO)

#endif // COUNTDTO_HPP