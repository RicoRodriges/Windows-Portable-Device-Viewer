#pragma once

#include "catch2/catch.hpp"

// Hack for Fakeit. I don't know about effects but it at least works
#ifndef CATCH_INTERNAL_UNSUPPRESS_PARENTHESES_WARNINGS
#define CATCH_INTERNAL_UNSUPPRESS_PARENTHESES_WARNINGS
#endif

#include "fakeit.hpp"

#include "custom_matchers.h"

#include "mock_wrappers.h"

#include "utils.h"