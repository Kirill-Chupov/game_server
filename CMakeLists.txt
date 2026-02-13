cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Группируем по модулям
set(HTTP_SERVER_MODULE
	src/http_server/http_server.h
	src/http_server/http_server.cpp
)

set(MODEL_MODULE 
	src/model/model.h
	src/model/model.cpp
	src/model/dog.h
	src/model/dog.cpp
	src/model/geometry.h
	src/model/loot_generator.h
	src/model/loot_generator.cpp
	src/model/collision_detector.h
	src/model/collision_detector.cpp
	src/model/collision_detector_adapters.h
	src/model/collision_detector_adapters.cpp
)

set(APP_MODULE
	src/app/application.h
	src/app/application.cpp
	src/app/player.h
	src/app/player.cpp
	src/app/player_tokens.h
	src/app/player_tokens.cpp
	src/app/players.h
	src/app/players.cpp
)

set(REQUEST_HANDLER_MODULE
	src/request_handler/request_handler.h
	src/request_handler/logging_request_handler.h
	src/request_handler/api_handler.h
	src/request_handler/api_handler.cpp
	src/request_handler/static_handler.h
	src/request_handler/static_handler.cpp
	src/request_handler/common_type.h
)

set(COMMON_MODULE
	src/common/sdk.h
	src/common/tagged.h
	src/common/tagged_uuid.h
	src/common/tagged_uuid.cpp
	src/common/boost_json.cpp
	src/common/json_loader.h
	src/common/json_loader.cpp
	src/common/logger.h
	src/common/logger.cpp
	src/common/parser_command_line.h
	src/common/ticker.h
	src/common/data_transfer_object.h
)

set(EXTRA_DATA_MODULE
	src/extra_data/extra_data.h
	src/extra_data/extra_data.cpp
)

set(STATE_MODULE
	src/state/state_serialization.h
	src/state/state_serialization.cpp
	src/state/state_file_io.h
	src/state/state_file_io.cpp
	src/state/auto_saver.h
	src/state/auto_saver.cpp
)

set(POSTGRES_MODULE
	src/postgres/postgres.h
	src/postgres/postgres.cpp
	src/postgres/unit_of_work.h
	src/postgres/repository.h
	src/postgres/use_cases.h
	src/postgres/use_cases_impl.h
	src/postgres/use_cases_impl.cpp
)

# Создаём статическую библиотеку
add_library(game_lib STATIC
	${MODEL_MODULE}
	${COMMON_MODULE}
	${POSTGRES_MODULE}
)

target_link_libraries(game_lib PUBLIC 
    Threads::Threads 
    CONAN_PKG::boost
	CONAN_PKG::libpq
	CONAN_PKG::libpqxx
)

# boost.beast будет использовать std::string_view вместо boost::string_view
target_compile_definitions(game_lib PUBLIC BOOST_BEAST_USE_STD_STRING_VIEW)

target_include_directories(game_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/http_server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/request_handler
	${CMAKE_CURRENT_SOURCE_DIR}/src/extra_data
	${CMAKE_CURRENT_SOURCE_DIR}/src/state
	${CMAKE_CURRENT_SOURCE_DIR}/src/postgres
)

add_executable(game_server
	src/main.cpp
	${HTTP_SERVER_MODULE}
	${APP_MODULE}
	${REQUEST_HANDLER_MODULE}
	${EXTRA_DATA_MODULE}
	${STATE_MODULE}
)

target_link_libraries(game_server PRIVATE game_lib)

option(BUILD_TESTS "Build tests" ON)
# Тесты - отдельная цель, которая не будет включаться в релиз
# Её можно собирать только при необходимости (например, для CI)
if(BUILD_TESTS)
	add_executable(game_server_tests
		tests/loot_generator_tests.cpp
		tests/collision_detector_tests.cpp
	)

	target_include_directories(game_server_tests PRIVATE 
		${CMAKE_CURRENT_SOURCE_DIR}/tests
	)

	target_link_libraries(game_server_tests PRIVATE
		CONAN_PKG::catch2
		game_lib
	)
endif()