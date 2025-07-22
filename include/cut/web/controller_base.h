#pragma once

#include <experimental/meta>
#include <format>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "utils/json.h"
#include "web/request.h"
#include "web/response.h"

using namespace std::literals;

namespace cut::web
{

/**
 * Base class for controllers that hides a lot of the reflection machinery.
 */
class ControllerBase
{
  public:
    virtual ~ControllerBase() = default;

    /**
     * Get the name of the derived class as a string.
     *
     *
     * @returns
     *   Name of derived class.
     */
    template <class Self>
    consteval auto name(this Self &&)
    {
        // decay the class name, the static_assert is to appease clang as it things T is unused...
        using T = std::decay_t<Self>;
        static_assert(sizeof(T) > 0);

        // getting the meta info of Self will be for the derived class
        // might be easier to just get this from the TypeList that lives in App - but I though this was a cute trick
        return std::meta::display_string_of(std::meta::dealias(^^T));
    }

    /**
     * Given a request try and find a matching method for it (has annotation and named correctly).
     *
     * @param request
     *   The request to try and handle.
     *
     * @returns
     *   An optional Response of the request could be handled.
     */
    template <class Self>
    auto dispatch_handler(this Self &&self, const Request &request) -> std::optional<Response>
    {
        // same as aboce
        using T = std::decay_t<Self>;
        static_assert(sizeof(T) > 0);

        constexpr auto ctx = std::meta::access_context::current();

        // get all the members of the derived class
        template for (constexpr auto member :
                      std::define_static_array(std::meta::members_of(std::meta::dealias(^^T), ctx)))
        {
            // filter out members that cannot be a handler
            if constexpr (
                std::meta::is_function(member) && !std::meta::is_special_member_function(member) &&
                !std::meta::is_constructor(member) && !std::meta::is_default_constructor(member))
            {
                // does the requested route match the method name?
                if (request.route == std::meta::display_string_of(member))
                {
                    constexpr auto annotations = std::define_static_array(std::meta::annotations_of(member));

                    if constexpr (!std::ranges::empty(annotations))
                    {
                        template for (constexpr auto annotation : annotations)
                        {
                            // does the annotation match the request method?
                            using A = typename[:std::meta::type_of(annotation):];
                            if (A::method == request.method)
                            {
                                // check if the function has params, if it does the parse it out from the body
                                constexpr auto params = std::define_static_array(std::meta::parameters_of(member));
                                if constexpr (std::ranges::size(params) == 1)
                                {
                                    // has function but not request body - something has gone wrong
                                    if (!request.body)
                                    {
                                        throw std::runtime_error(std::format("missing body for {}", request));
                                    }

                                    // convert the json int the body into the type the function is expecting
                                    auto arg =
                                        utils::from_json<typename[:std::meta::type_of(params.front()):]>(*request.body);

                                    // splice a meta::info into deducing this member call, to get a value via a promise
                                    // from a returned coroutine - obviously
                                    return {self.[:member:](std::move(arg)).native_handle().promise().value};
                                }
                                else
                                {
                                    // splice a meta::info into deducing this member call, to get a value via a promise
                                    // from a returned coroutine - obviously
                                    return {self.[:member:]().native_handle().promise().value};
                                }
                            }
                        }
                    }
                }
            }
        }

        return std::nullopt;
    }
};

}
