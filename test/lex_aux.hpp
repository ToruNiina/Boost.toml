#define TOML_LEX_CHECK_ACCEPT(lxr, tkn, expct) {\
    const std::string token(tkn);\
    const std::string expect(expct);\
    std::string::const_iterator iter = token.begin();\
    const boost::optional<std::string> result = \
        lxr::invoke(iter, token.end());\
    BOOST_CHECK(result);\
    if(result){BOOST_CHECK_EQUAL(*result, expect);}\
    BOOST_CHECK(iter == token.begin() + expect.size());\
} /**/

#define TOML_LEX_CHECK_REJECT(lxr, tkn) {\
    const std::string token(tkn);\
    std::string::const_iterator iter = token.begin();\
    const boost::optional<std::string> result = \
        lxr::invoke(iter, token.end());\
    BOOST_CHECK(!result);\
    if(result){std::cerr << "result = " << *result << std::endl;}\
    BOOST_CHECK(iter != token.end());\
} /**/
