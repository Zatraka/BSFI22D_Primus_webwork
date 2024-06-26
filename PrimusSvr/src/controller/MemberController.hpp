#ifndef MEMBERCONTROLLER_HPP
#define MEMBERCONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "dto/StatusDto.hpp"
#include "dto/PageDto.hpp"
#include "dto/Int32Dto.hpp"
#include "dto/BooleanDto.hpp"
#include "general/constants.hpp"
#include "assert.h"

namespace primus {
    namespace apicontroller {
        namespace member_endpoint {

#include OATPP_CODEGEN_BEGIN(ApiController) // Begin API Controller codegen


            using primus::dto::PageDto;
            //  __  __                _                ____            _             _ _           
            // |  \/  | ___ _ __ ___ | |__   ___ _ __ / ___|___  _ __ | |_ _ __ ___ | | | ___ _ __ 
            // | |\/| |/ _ \ '_ ` _ \| '_ \ / _ \ '__| |   / _ \| '_ \| __| '__/ _ \| | |/ _ \ '__|
            // | |  | |  __/ | | | | | |_) |  __/ |  | |__| (_) | | | | |_| | | (_) | | |  __/ |   
            // |_|  |_|\___|_| |_| |_|_.__/ \___|_|   \____\___/|_| |_|\__|_|  \___/|_|_|\___|_|   

            class MemberController : public oatpp::web::server::api::ApiController
            {
                typedef primus::dto::database::MemberDto MemberDto;
                typedef primus::dto::database::DepartmentDto DepartmentDto;
                typedef primus::dto::database::AddressDto AddressDto;
                typedef primus::dto::database::DateDto DateDto;
                typedef primus::dto::MemberPageDto MemberPageDto;
                typedef primus::dto::UInt32Dto UInt32Dto;
                typedef primus::dto::Int32Dto Int32Dto;
                typedef primus::dto::BooleanDto BooleanDto;
                typedef primus::dto::StatusDto StatusDto;

            private:
                OATPP_COMPONENT(std::shared_ptr<primus::component::DatabaseClient>, m_database);

            public:
                MemberController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
                    : oatpp::web::server::api::ApiController(objectMapper)
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "MemberController (oatpp::web::server::api::ApiController) initialized");
                    
                }

                static std::shared_ptr<MemberController> createShared(
                    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
                )
                {
                    return std::make_shared<MemberController>(objectMapper);
                }

                ENDPOINT("GET", "/api/members/list/{attribute}", getMembersList,
                    PATH(oatpp::String, attribute), QUERY(oatpp::UInt32, limit), QUERY(oatpp::UInt32, offset))
                {
                    
                    std::shared_ptr<oatpp::orm::QueryResult> dbResult;
                    if (attribute == oatpp::String("all"))
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list of all members. Limit: %d, Offset: %d", limit.operator v_uint32(), offset.operator v_uint32());

                        dbResult = m_database->getAllMembers(limit, offset);
                    }
                    else if (attribute == oatpp::String("active"))
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list of all active members. Limit: %d, Offset: %d", limit.operator v_uint32(), offset.operator v_uint32());

                        dbResult = m_database->getActiveMembers(limit, offset);
                    }
                    else if (attribute == oatpp::String("inactive"))
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list of all inactive members. Limit: %d, Offset: %d", limit.operator v_uint32(), offset.operator v_uint32());

                        dbResult = m_database->getInactiveMembers(limit, offset);
                    }
                    else if (attribute == oatpp::String("birthday"))
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list of all members with upcomming birthdays. Limit: %d, Offset: %d", limit.operator v_uint32(), offset.operator v_uint32());

                        dbResult = m_database->getMembersWithUpcomingBirthday(limit, offset);
                    }
                    else
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list of members with %s, which is not an available attribute", attribute->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Returning CODE 404: NOT FOUND. Available options: birthday, all, active or inactive");

                        auto status = primus::dto::StatusDto::createShared();
                        status->code = 404;
                        status->message = "Received request to get a list of members with invalid attribute. Available options: birthday, all, active or inactive";
                        status->status = "INVALID ATTRIBUTE";
                        return createDtoResponse(Status::CODE_200, status);
                    }
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();

                    auto page = PageDto<oatpp::Object<MemberDto>>::createShared();

                    page->offset = offset;
                    page->limit = limit;
                    page->count = items->size();
                    page->items = items;

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Processed request to get a list of members with %s. Limit: %d, Offset: %d. Returned %d items", attribute->c_str(), limit.operator v_uint32(), offset.operator v_uint32(), page->count.operator v_uint32());
                    
                    return createDtoResponse(Status::CODE_200, page);
                }

                ENDPOINT("UPDATE", "/api/member/{id}/activate", activateMember,
                    PATH(oatpp::UInt32, id))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to activate member with id: %d", id);

                    {
                        auto status = primus::assert::assertMemberExists(id);

                        if (status->code != 200)
                        {
                            return createDtoResponse(Status::CODE_500, status);
                        }
                    }

                    auto dbResult = m_database->activateMember(id);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Unknown error");

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member with id: %d activated", id);
                    

                    auto status = primus::dto::StatusDto::createShared();
                    status->code = 200;
                    status->message = "Member has been activated";
                    status->status = "Member activated";
                    return createDtoResponse(Status::CODE_200, status);
                }

                ENDPOINT("UPDATE", "/api/member/{id}/deactivate", deactivateMember,
                    PATH(oatpp::UInt32, id))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to deactivate member with id: %d", id);

                    {
                        auto status = primus::assert::assertMemberExists(id);
                        if (status->code != 200)
                        {
                            return createDtoResponse(Status::CODE_500, status);
                        }
                    }

                    auto dbResult = m_database->deactivateMember(id);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "UNKNOWN ERROR");

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member with id: %d deactivated", id);
                    

                    auto status = primus::dto::StatusDto::createShared();
                    status->code = 200;
                    status->message = "Member has been deactivated";
                    status->status = "Member deactivated";
                    return createDtoResponse(Status::CODE_200, status);
                }

                ENDPOINT("GET", "/api/member/{id}", getMemberById,
                    PATH(oatpp::UInt32, id))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get member by id: %d", id.operator v_uint32());

                    auto status = primus::assert::assertMemberExists(id);

                    if (status->code != 200)
                    {
                        return createDtoResponse(Status::CODE_500, status);
                    }

                    auto dbResult = m_database->getMemberById(id);

                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "Member not found");

                    auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
                    OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Processed request to get member by id: %d", id.operator v_uint32());
                    
                    return createDtoResponse(Status::CODE_200, result);
                }

                ENDPOINT("POST", "/api/member", createMember,
                    BODY_DTO(Object<MemberDto>, member))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to create member");

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member data:");
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - First Name: %s", member->firstName->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Last Name: %s", member->lastName->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Email: %s", member->email->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Phone Number: %s", member->phoneNumber->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Birth Date: %s", member->birthDate->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Create Date: %s", member->createDate->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Notes: %s", member->notes->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Active: %s", member->active ? "true" : "false");

                    std::shared_ptr<oatpp::orm::QueryResult> dbResult = m_database->createMember(member);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Bad Request");

                    oatpp::UInt32 memberId = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());

                    oatpp::Vector<oatpp::Object<MemberDto>> foundMembers;
                    oatpp::Object<MemberDto> retMember;

                    if (memberId == 0)
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member already exists. proceeding to return existing user");

                        dbResult = m_database->findMemberIdByDetails(member->firstName, member->lastName, member->email, member->birthDate);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Unknown error");

                        foundMembers = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();

                        retMember = foundMembers[0];
                    }
                    else
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Created member with id: %d", memberId.operator v_uint32());

                        dbResult = m_database->getMemberById(memberId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Unknown error");

                        foundMembers = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();

                        retMember = foundMembers[0];
                    }
                    
                    return createDtoResponse(memberId == 0 ? Status::CODE_200 : Status::CODE_201, retMember);
                }

                ENDPOINT("PUT", "/api/member", updateMember,
                    BODY_DTO(Object<MemberDto>, member))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to update member with id: %d", member->id.operator v_uint32());

                    {
                        auto dbResult = m_database->getMemberById(member->id);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                        auto memberArray = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
                        auto currentMember = memberArray[0];

                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Old member data:");
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - ID: %d", currentMember->id.operator v_uint32());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - First Name: %s", currentMember->firstName->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Last Name: %s", currentMember->lastName->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Email: %s", currentMember->email->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Phone Number: %s", currentMember->phoneNumber->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Birth Date: %s", currentMember->birthDate->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Create Date: %s", currentMember->createDate->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Notes: %s", currentMember->notes->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Active: %s", currentMember->active ? "true" : "false");

                        if (member->firstName == nullptr || member->lastName == nullptr || member->email == nullptr || member->phoneNumber == nullptr || member->birthDate == nullptr || member->notes == nullptr)
                        {
                            oatpp::Object<primus::dto::StatusDto> status = primus::dto::StatusDto::createShared();

                            status->code = 403;
                            status->message = "At least one necessary field was null";
                            status->status = "At least one necessary field was null";

                            return createDtoResponse(Status::CODE_403, status);
                        }

                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "New member data:");
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - ID: %d", member->id.operator v_uint32());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - First Name: %s", member->firstName->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Last Name: %s", member->lastName->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Email: %s", member->email->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Phone Number: %s", member->phoneNumber->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Birth Date: %s", member->birthDate->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Create Date: %s", member->createDate->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Notes: %s", member->notes->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "  - Active: %s", member->active ? "true" : "false");
                    }

                    auto dbResult = m_database->updateMember(member);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Updated member with id: %d", member->id.operator v_uint32());
                    
                    return getMemberById(member->id);
                }

                ENDPOINT("GET", "/api/members/count/{attribute}", getMemberCount, PATH(oatpp::String, attribute))
                {
                    

                    std::shared_ptr<oatpp::orm::QueryResult> dbResult;

                    if (attribute == oatpp::String("all"))
                    {
                        dbResult = m_database->getMemberCountAll();
                    }
                    else if (attribute == oatpp::String("active"))
                    {
                        dbResult = m_database->getMemberCountActive();
                    }
                    else if (attribute == oatpp::String("inactive"))
                    {
                        dbResult = m_database->getMemberCountInactive();
                    }
                    else
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received count request for members with attribute %s, which is not an available attribute", attribute->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Returning CODE 500: Bad Request. Available options: all, active, inactive");

                        auto status = primus::dto::StatusDto::createShared();
                        status->code = 404;
                        status->message = "Received request for a count of members with invalid attribute. Available options: all, active, inactive";
                        status->status = "INVALID ATTRIBUTE";
                        return createDtoResponse(Status::CODE_404, status);
                    }

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get count of %s members", attribute->c_str());
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    auto count = dbResult->fetch<oatpp::Vector<oatpp::Object<UInt32Dto>>>();

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Processed request to get count of %s members. Total count: %d", attribute->c_str(), count[0]->value.operator v_uint32());
                    
                    return createDtoResponse(Status::CODE_200, count[0]);
                }

                ENDPOINT("POST", "/api/member/{memberId}/department/add/{departmentId}", createMemberDepartmentAssociation, PATH(oatpp::UInt32, memberId), PATH(oatpp::UInt32, departmentId))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to add member with id %d to department with id %d", memberId.operator v_uint32(), departmentId.operator v_uint32());

                    std::shared_ptr<OutgoingResponse>           ret;
                    oatpp::Vector<oatpp::Object<DepartmentDto>> departments;
                    oatpp::Vector<oatpp::Object<MemberDto>>     member;
                    std::shared_ptr<oatpp::orm::QueryResult>    dbResult;


                    dbResult = m_database->getDepartmentById(departmentId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    departments = dbResult->fetch<oatpp::Vector<oatpp::Object<DepartmentDto>>>();
                    OATPP_ASSERT_HTTP(departments->size() != 0, Status::CODE_404, "Department not found");
                    OATPP_ASSERT_HTTP(!(departments->size() > 1), Status::CODE_404, "Critical database error: More than 1 department with id %d", departmentId.operator v_uint32());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Department found: %d | %s", departments[0]->id.operator v_uint32(), departments[0]->name->c_str());



                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Creating member-department association");
                    dbResult = m_database->associateDepartmentWithMember(departmentId, memberId);

                    if (!dbResult->isSuccess())
                    {
                        oatpp::Object<primus::dto::StatusDto> ret = primus::dto::StatusDto::createShared();

                        oatpp::String verboseMessage = "Database operation did not succeed. This might be because the member is already associated with the department. The database error message was the following: ";
                        verboseMessage->append(dbResult->getErrorMessage());

                        ret->code = 500;
                        ret->message = verboseMessage;
                        ret->status = "Member likely already in department";

                        return createDtoResponse(Status::CODE_500, ret);
                    }
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "member-department association successfully created");

                    auto status = primus::dto::StatusDto::createShared();
                    status->code = 200;
                    status->message = "member-department association successfully created";
                    status->status = "member-department association successfully created";
                    return createDtoResponse(Status::CODE_200, status);
                }
                ENDPOINT("DELETE", "/api/member/{memberId}/department/remove/{departmentId}", deleteMemberDepartmentDisassociation, PATH(oatpp::UInt32, memberId), PATH(oatpp::UInt32, departmentId))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to remove member with id %d from department with id %d", memberId.operator v_uint32(), departmentId.operator v_uint32());

                    std::shared_ptr<OutgoingResponse>           ret;
                    oatpp::Vector<oatpp::Object<DepartmentDto>> departments;
                    oatpp::Vector<oatpp::Object<MemberDto>>     member;
                    std::shared_ptr<oatpp::orm::QueryResult>    dbResult;


                    dbResult = m_database->getDepartmentById(departmentId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    departments = dbResult->fetch<oatpp::Vector<oatpp::Object<DepartmentDto>>>();
                    OATPP_ASSERT_HTTP(departments->size() != 0, Status::CODE_404, "Department not found");
                    OATPP_ASSERT_HTTP(!(departments->size() > 1), Status::CODE_404, "Critical database error: More than 1 department with id %d", departmentId.operator v_uint32());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Department found: %d | %s", departments[0]->id.operator v_uint32(), departments[0]->name->c_str());

                    auto memberStatus = primus::assert::assertMemberExists(memberId);
                    if (memberStatus->code != 200)
                        return createDtoResponse(Status::CODE_500, memberStatus);

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Disassociating member and department");
                    dbResult = m_database->disassociateDepartmentFromMember(departmentId, memberId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "member and department successfully disassociated");

                    memberStatus = primus::dto::StatusDto::createShared();

                    memberStatus->code = 200;
                    memberStatus->message = "member and department successfully disassociated";
                    memberStatus->status = "member and department successfully disassociated";

                    return createDtoResponse(Status::CODE_200, memberStatus);
                }

                ENDPOINT("POST", "/api/member/{memberId}/address/add", createMemberAddressAssociation, PATH(oatpp::UInt32, memberId), BODY_DTO(oatpp::Object<AddressDto>, address))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to set address for member with id %d", memberId.operator v_uint32());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Address data:");
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "- Street: %s", address->street->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "- City: %s", address->city->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "- Postal code: %s", address->postalCode->c_str());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "- Country: %s", address->country->c_str());

                    auto status = primus::assert::assertMemberExists(memberId);
                    if (status->code != 200)
                    {
                        return createDtoResponse(Status::CODE_500, status);
                    }

                    auto dbResult = m_database->createAddress(address);

                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    oatpp::UInt32 addressId = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());

                    oatpp::Vector<oatpp::Object<AddressDto>> foundAddresses;
                    oatpp::Object<AddressDto> retAddress;

                    if (addressId == 0)
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Address already exists. Proceeding to return existing id");

                        dbResult = m_database->findAddressByDetails(address->street, address->city, address->postalCode, address->country);
                    }
                    else
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Address created with id %d", addressId.operator v_uint32());

                        dbResult = m_database->getAddressById(addressId);
                    }
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Unknown Error");

                    foundAddresses = dbResult->fetch<oatpp::Vector<oatpp::Object<AddressDto>>>();
                    retAddress = foundAddresses[0];

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Creating member-address association");
                    dbResult = m_database->associateAddressWithMember(retAddress->id, memberId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, "Unknown Error");
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "member-address association was successfully created");

                    return createDtoResponse(addressId == 0 ? Status::CODE_200 : Status::CODE_201, retAddress);
                }
                ENDPOINT("DELETE", "/api/member/{memberId}/address/remove/{addressId}", deleteMemberAddressDisassociation, PATH(oatpp::UInt32, memberId), PATH(oatpp::UInt32, addressId))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to remove member with id %d from address with id %d", memberId.operator v_uint32(), addressId.operator v_uint32());

                    std::shared_ptr<OutgoingResponse>           ret;
                    oatpp::Vector<oatpp::Object<AddressDto>>    addresses;
                    oatpp::Vector<oatpp::Object<MemberDto>>     member;
                    std::shared_ptr<oatpp::orm::QueryResult>    dbResult;


                    dbResult = m_database->getAddressById(addressId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    addresses = dbResult->fetch<oatpp::Vector<oatpp::Object<AddressDto>>>();
                    OATPP_ASSERT_HTTP(addresses->size() != 0, Status::CODE_404, "address not found");
                    OATPP_ASSERT_HTTP(!(addresses->size() > 1), Status::CODE_404, "Critical database error: More than 1 address with id %d", departmentId.operator v_uint32());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Address found");

                    {
                        auto status = primus::assert::assertMemberExists(memberId);
                        if (status->code != 200)
                        {
                            return createDtoResponse(Status::CODE_500, status);
                        }
                    }

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Disassociating member and address");
                    dbResult = m_database->disassociateAddressFromMember(addressId, memberId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "member and department successfully disassociated");

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Checking for other members using the address...");
                    dbResult = m_database->getMembersByAddress(addressId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    member = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();


                    if (member->size() == 0)
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "No other member using the address.");
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Proceeding with deletion of address");

                        dbResult = m_database->deleteAddress(addressId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Address has been deleted");
                    }
                    else
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Other members are associated with address");
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "keeping address in database");
                    }
                    auto status = primus::dto::StatusDto::createShared();
                    status->code = 200;
                    status->message = "member and department successfully disassociated";
                    status->status = "member and department successfully disassociated";
                    return createDtoResponse(Status::CODE_200, status);
                }

                ENDPOINT("POST", "/api/member/{memberId}/attendance/{dateOfAttendance}", addMemberAttendance, PATH(oatpp::UInt32, memberId), PATH(oatpp::String, dateOfAttendance))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request set member attendance for member with id %d", memberId.operator v_uint32());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Date of attendance: %s", dateOfAttendance->c_str());

                    std::shared_ptr<oatpp::orm::QueryResult> dbResult = m_database->getMemberById(memberId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    oatpp::Vector<oatpp::Object<MemberDto>> members = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
                    OATPP_ASSERT_HTTP(members->size() > 0, Status::CODE_404, "Member not found");
                    OATPP_ASSERT_HTTP(members->size() < 2, Status::CODE_500, "Critical database error: more than one member with given id");

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member found");

                    dbResult = m_database->createMemberAttendance(memberId, dateOfAttendance);
                    auto foo = dbResult->getErrorMessage();
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member attendance was set for date %s", dateOfAttendance->c_str());

                    auto status = primus::dto::StatusDto::createShared();
                    status->code = 200;
                    status->message = "Attendance set";
                    status->status = "Attendance set";
                    return createDtoResponse(Status::CODE_200, status);
                }
                ENDPOINT("DELETE", "/api/member/{memberId}/attendance/{dateOfAttendance}", deleteMemberAttendance, PATH(oatpp::UInt32, memberId), PATH(oatpp::String, dateOfAttendance))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request remove member attendance for member with id %d", memberId.operator v_uint32());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Date of attendance: %s", dateOfAttendance->c_str());

                    std::shared_ptr<oatpp::orm::QueryResult> dbResult = m_database->getMemberById(memberId); // Wheather or not the member exists
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    auto members = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
                    OATPP_ASSERT_HTTP(members->size() > 0, Status::CODE_404, "Member not found");
                    OATPP_ASSERT_HTTP(members->size() < 2, Status::CODE_500, "Critical database error: more than one member with given id");

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member found");

                    dbResult = m_database->deleteMemberAttendance(memberId, dateOfAttendance);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member attendance was removed for date %s", dateOfAttendance->c_str());

                    auto status = primus::dto::StatusDto::createShared();
                    status->code = 200;
                    status->message = "Attendance removed";
                    status->status = "Attendance removed";
                    return createDtoResponse(Status::CODE_200, status);
                }

                ENDPOINT("GET", "/api/member/{memberId}/fee", getMemberFee,
                    PATH(oatpp::UInt32, memberId))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to calculate the member fee for member with id %d.", memberId.operator v_uint32());

                    std::shared_ptr<oatpp::orm::QueryResult> dbResult;
                    oatpp::Vector<oatpp::Object<MemberDto>> members;
                    oatpp::Vector<oatpp::Object<DepartmentDto>> departments;
                    auto memberFee = UInt32Dto::createShared();

                    dbResult = m_database->getMemberById(memberId);
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                    members = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
                    OATPP_ASSERT_HTTP(members->size() > 0, Status::CODE_404, "Member not found");
                    OATPP_ASSERT_HTTP(members->size() < 2, Status::CODE_500, "Critical database error: more than one member with given id");

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member was found.", memberId.operator v_uint32());

                    dbResult = m_database->getMemberDepartments(memberId, oatpp::UInt32(3), oatpp::UInt32(static_cast<unsigned int>(0)));
                    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                    departments = dbResult->fetch<oatpp::Vector<oatpp::Object<DepartmentDto>>>();

                    if (departments->size() < 1) // No department
                        memberFee->value = primus::constants::pricing::DepartmentPrices::None;
                    else if (departments->size() > 1) // multiple departments
                        memberFee->value = primus::constants::pricing::DepartmentPrices::Multiple;
                    else if (departments->size() == 1) // Only one department
                        switch (departments[0]->id)
                        {
                        case 1: // Bogenschiessen
                            memberFee->value = primus::constants::pricing::DepartmentPrices::Bogenschiessen;
                            break;
                        case 2: // Luftdruck
                            memberFee->value = primus::constants::pricing::DepartmentPrices::Luftdruck;
                            break;
                        case 3: // Schusswaffen
                            memberFee->value = primus::constants::pricing::DepartmentPrices::Schusswaffen;
                            break;
                        }
                    else
                    {
                        auto status = primus::dto::StatusDto::createShared();

                        status->status = "UNKNOWN ERROR";
                        status->code = 500;
                        status->message = "Failed while trying to determine membership fee";
                        return createDtoResponse(Status::CODE_500, status);
                    }

                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member is in %d departments.", departments->size());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Fee is %d euro", memberFee->value.operator v_uint32());

                    

                    return createDtoResponse(Status::CODE_200, memberFee);
                }

                ENDPOINT("GET", "/api/member/{memberId}/list/{attribute}", getMemberList,
                    PATH(oatpp::UInt32, memberId), PATH(oatpp::String, attribute), QUERY(oatpp::UInt32, limit), QUERY(oatpp::UInt32, offset))
                {
                    

                    std::shared_ptr<oatpp::orm::QueryResult> dbResult;
                    std::shared_ptr<OutgoingResponse> ret;

                    {
                        std::shared_ptr<oatpp::orm::QueryResult> dbResult = m_database->getMemberById(memberId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                        oatpp::Vector<oatpp::Object<MemberDto>> members = dbResult->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
                        OATPP_ASSERT_HTTP(members->size() > 0, Status::CODE_404, "Member not found");
                        OATPP_ASSERT_HTTP(members->size() < 2, Status::CODE_500, "Critical database error: more than one member with given id");
                    }

                    if (attribute == oatpp::String("addresses"))
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list addresses associated with member id %d. Limit: %d, Offset: %d", memberId.operator v_uint32(), limit.operator v_uint32(), offset.operator v_uint32());

                        dbResult = m_database->getMemberAddresses(memberId, limit, offset);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<AddressDto>>>();

                        auto page = PageDto<oatpp::Object<AddressDto>>::createShared();

                        page->offset = offset;
                        page->limit = limit;
                        page->count = items->size();
                        page->items = items;

                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Processed request to get a list of members with %s. Limit: %d, Offset: %d. Returned %d items", attribute->c_str(), limit.operator v_uint32(), offset.operator v_uint32(), page->count.operator v_uint32());

                        ret = createDtoResponse(Status::CODE_200, page);
                    }
                    else if (attribute == oatpp::String("departments"))
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list departments associated with member id %d. Limit: %d, Offset: %d", memberId.operator v_uint32(), limit.operator v_uint32(), offset.operator v_uint32());

                        dbResult = m_database->getMemberDepartments(memberId, limit, offset);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<DepartmentDto>>>();


                        auto page = PageDto<oatpp::Object<DepartmentDto>>::createShared();

                        page->offset = offset;
                        page->limit = limit;
                        page->count = items->size();
                        page->items = items;

                        ret = createDtoResponse(Status::CODE_200, page);
                    }
                    else if (attribute == oatpp::String("attendances"))
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list attendances associated with member id %d. Limit: %d, Offset: %d", memberId.operator v_uint32(), limit.operator v_uint32(), offset.operator v_uint32());

                        dbResult = m_database->getAttendancesOfMember(memberId, limit, offset);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

                        auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<DateDto>>>();

                        auto page = PageDto<oatpp::Object<DateDto>>::createShared();

                        page->offset = offset;
                        page->limit = limit;
                        page->count = items->size();
                        page->items = items;

                        ret = createDtoResponse(Status::CODE_200, page);
                    }
                    else
                    {
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to get a list of members with %s, which is not an available attribute", attribute->c_str());
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Returning CODE 500: Bad Request. Available options: addresses");

                        auto status = primus::dto::StatusDto::createShared();

                        oatpp::String verboseMessage = "Received request to get a list of members with invalid attribute: ";
                        verboseMessage->append(attribute);


                        status->code = 404;
                        status->message = verboseMessage;
                        status->status = "INVALID ATTRIBUTE";
                        return createDtoResponse(Status::CODE_404, status);
                    }

                    

                    return ret;
                }

                ENDPOINT("GET", "api/member/{memberId}/weaponpurchase/", canMemberBuyWeapon,
                    PATH(oatpp::UInt32, memberId))
                {
                    
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Received request to check if member with id %d", memberId.operator v_uint32());
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "is allowed to purchase a weapon");
                    {
                        auto status = primus::assert::assertMemberExists(memberId);

                        if (status->code != 200)
                            return createDtoResponse(Status::CODE_500, status);
                    }
                    OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member was found");

                    {
                        std::shared_ptr<oatpp::orm::QueryResult> dbResult;
                        oatpp::Vector<oatpp::Object<UInt32Dto>> count;
                        auto ret = BooleanDto::createShared();

                        dbResult = m_database->getCountOfMemberAttendancesInLastYear(memberId);
                        OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                        count = dbResult->fetch<oatpp::Vector<oatpp::Object<UInt32Dto>>>();

                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Checking first condition of weapon purchase...");
                        OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member attended %d sessions last year.", count[0]->value.operator v_uint32());

                        if (count[0]->value >= 18)
                        {
                            ret->value = true;
                            OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Which allowes him to purchase a weapon");
                            OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Returning true");
                        }
                        else
                        {
                            OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member does not have the yearly attendance to purchase a weapon");
                            OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Checking for secondary-condition (monthly attendance x1)");

                            dbResult = m_database->countDistinctAttendentMontsWithinLastYear(memberId);
                            OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
                            count = dbResult->fetch<oatpp::Vector<oatpp::Object<UInt32Dto>>>();

                            if (count[0]->value == 12)
                            {
                                OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member attended at least one session per month");
                                OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Returning true");
                                ret->value = true;
                            }
                            else
                            {
                                OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Member attended less than one session per month");
                                OATPP_LOGI(primus::constants::apicontroller::member_endpoint::logName, "Returning false");
                                ret->value = false;
                            }
                        }

                        return createDtoResponse(Status::CODE_200, ret);
                    }
                }
                // Endpoint Infos

                ENDPOINT_INFO(getMembersList)
                {
                    info->name = "getMembersList";
                    info->summary = "Get a list of members based on attribute";
                    info->description = "This endpoint retrieves a list of members based on the provided attribute. Available attributes are: all, active, inactive, birthday.";
                    info->path = "/api/members/list/{attribute}";
                    info->method = "GET";
                    info->addTag("Members");
                    info->addTag("List");
                    info->pathParams["attribute"].description = "Attribute to filter members (options: all, active, inactive, birthday)";
                    info->queryParams["limit"].description = "Maximum number of items to return";
                    info->queryParams["offset"].description = "Number of items to skip before starting to collect the response items";
                    info->addResponse<oatpp::Object<PageDto<oatpp::Vector<oatpp::Object<MemberDto>>>>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(activateMember) {
                    info->name = "activateMember";
                    info->summary = "Activate a member by ID";
                    info->description = "This endpoint activates a member with the provided ID.";
                    info->path = "/api/member/{id}/activate";
                    info->method = "UPDATE";
                    info->addTag("Member");
                    info->pathParams["id"].description = "Identifier of the member to activate";
                    info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(deactivateMember) {
                    info->name = "deactivateMember";
                    info->summary = "Deactivate a member by ID";
                    info->description = "This endpoint deactivates a member with the provided ID.";
                    info->path = "/api/member/{id}/deactivate";
                    info->method = "UPDATE";
                    info->addTag("Member");
                    info->pathParams["id"].description = "Identifier of the member to deactivate";
                    info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(getMemberById) {
                    info->name = "getMemberById";
                    info->summary = "Get a member by ID";
                    info->description = "This endpoint retrieves a member with the provided ID.";
                    info->path = "/api/member/{id}";
                    info->method = "GET";
                    info->addTag("Member");
                    info->pathParams["id"].description = "Identifier of the member to retrieve";
                    info->addResponse<oatpp::Vector<oatpp::Object<MemberDto>>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(createMember)
                {
                    info->name = "createMember";
                    info->summary = "Create a new member";
                    info->description = "This endpoint creates a new member with the provided data.";
                    info->path = "/api/member";
                    info->method = "POST";
                    info->addTag("Member");
                    info->bodyContentType = "application/json";
                    info->addResponse<oatpp::Object<MemberDto>>(Status::CODE_200, "application/json");
                    info->addResponse<oatpp::Object<MemberDto>>(Status::CODE_201, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(updateMember)
                {
                    info->name = "updateMember";
                    info->summary = "Update an existing member";
                    info->description = "This endpoint updates an existing member with the provided data.";
                    info->path = "/api/member";
                    info->method = "PUT";
                    info->addTag("Member");
                    info->bodyContentType = "application/json";
                    info->addResponse<oatpp::Object<MemberDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(getMemberCount)
                {
                    info->name = "getMemberCount";
                    info->summary = "Get the count of members based on attribute";
                    info->description = "This endpoint retrieves the count of members based on the provided attribute. Available attributes are: all, active, inactive.";
                    info->path = "/api/members/count/{attribute}";
                    info->method = "GET";
                    info->addTag("Members");
                    info->addTag("Counts");
                    info->queryParams["attribute"].description = "Attribute to filter members (options: all, active, inactive)";
                    info->addResponse<Object<UInt32Dto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(createMemberDepartmentAssociation)
                {
                    info->name = "createMemberDepartmentAssociation";
                    info->summary = "Create association between member and department";
                    info->description = "This endpoint creates an association between a member and a department.";
                    info->path = "/api/member/{memberId}/department/add/{departmentId}";
                    info->method = "POST";
                    info->addTag("Member");
                    info->addTag("Department");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->pathParams["departmentId"].description = "ID of the department";
                    info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(deleteMemberDepartmentDisassociation)
                {
                    info->name = "deleteMemberDepartmentDisassociation";
                    info->summary = "Remove association between member and department";
                    info->description = "This endpoint removes the association between a member and a department.";
                    info->path = "/api/member/{memberId}/department/remove/{departmentId}";
                    info->method = "DELETE";
                    info->addTag("Member");
                    info->addTag("Department");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->pathParams["departmentId"].description = "ID of the department";
                    info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(createMemberAddressAssociation)
                {
                    info->name = "createMemberAddressAssociation";
                    info->summary = "Create association between member and address";
                    info->description = "This endpoint creates an association between a member and an address.";
                    info->path = "/api/member/{memberId}/address/add";
                    info->method = "POST";
                    info->addTag("Member");
                    info->addTag("Address");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->addResponse<Object<AddressDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<AddressDto>>(Status::CODE_201, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(deleteMemberAddressDisassociation)
                {
                    info->name = "deleteMemberAddressDisassociation";
                    info->summary = "Remove association between member and address";
                    info->description = "This endpoint removes the association between a member and an address.";
                    info->path = "/api/member/{memberId}/address/remove/{addressId}";
                    info->method = "DELETE";
                    info->addTag("Member");
                    info->addTag("Address");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->pathParams["addressId"].description = "ID of the address";
                    info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(addMemberAttendance)
                {
                    info->name = "addMemberAttendance";
                    info->summary = "Add attendance for a member on a specific date";
                    info->description = "This endpoint adds attendance for a member on a specific date.";
                    info->path = "/api/member/{memberId}/attendance/{dateOfAttendance}";
                    info->method = "POST";
                    info->addTag("Member");
                    info->addTag("Attendance");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->pathParams["dateOfAttendance"].description = "Date of the attendance (format: YYYY-MM-DD)";
                    info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(deleteMemberAttendance)
                {
                    info->name = "deleteMemberAttendance";
                    info->summary = "Remove attendance for a member on a specific date";
                    info->description = "This endpoint removes attendance for a member on a specific date.";
                    info->path = "/api/member/{memberId}/attendance/{dateOfAttendance}";
                    info->method = "DELETE";
                    info->addTag("Member");
                    info->addTag("Attendance");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->pathParams["dateOfAttendance"].description = "Date of the attendance (format: YYYY-MM-DD)";
                    info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(getMemberFee)
                {
                    info->name = "getMemberFee";
                    info->summary = "Calculate the member fee for a member";
                    info->description = "This endpoint calculates the membership fee for a member based on their department.";
                    info->path = "/api/member/{memberId}/fee";
                    info->method = "GET";
                    info->addTag("Member");
                    info->addTag("Fee");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->addResponse<Object<UInt32Dto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(getMemberList)
                {
                    info->name = "getMemberList";
                    info->summary = "Get a list of information associated with a member";
                    info->description = "This endpoint retrieves a list of information associated with a member, such as addresses, departments, or attendances.";
                    info->path = "/api/member/{memberId}/list/{attribute}";
                    info->method = "GET";
                    info->addTag("Member");
                    info->addTag("List");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->pathParams["attribute"].description = "Attribute to retrieve (addresses, departments, attendances)";
                    info->queryParams["limit"].description = "Limit of items to retrieve (default is 0)";
                    info->queryParams["offset"].description = "Offset for pagination (default is 0)";
                    info->addResponse<Object<PageDto<oatpp::Object<AddressDto>>>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<PageDto<oatpp::Object<DepartmentDto>>>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<PageDto<oatpp::Object<DateDto>>>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }

                ENDPOINT_INFO(canMemberBuyWeapon)
                {
                    info->name = "canMemberBuyWeapon";
                    info->summary = "Check if a member is allowed to purchase a weapon";
                    info->description = "This endpoint checks if a member is eligible to purchase a weapon based on their attendance records.";
                    info->path = "/api/member/{memberId}/weaponpurchase/";
                    info->method = "GET";
                    info->addTag("Member");
                    info->addTag("Weapon");
                    info->pathParams["memberId"].description = "ID of the member";
                    info->addResponse<Object<BooleanDto>>(Status::CODE_200, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
                    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
                }


            };

#include OATPP_CODEGEN_END(ApiController) // End API Controller codegen

        } // namespace member_endpoint
    } // apicontroller
} // namespace namespace primus

#endif // MEMBERCONTROLLER_HPP
