#include "log.hpp"

int main()
{
    logger::init("elution_profiling", logger::severity_level::debug);
    logger::info("The definition ", "of"," insanity.");
    logger::warning("The definition ", "of"," insanity.");
    logger::debug("The definition ", "of"," insanity.");
    logger::error("The definition ", "of"," insanity.");
    logger::fatal("The definition ", "of"," insanity.");
}