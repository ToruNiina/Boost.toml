#define TOML_PARSE_CHECK_EQUAL(psr, tkn, tp, expct) {\
    const std::string token(tkn);\
    std::string::const_iterator iter = token.begin();\
    const result<tp, std::string> r = psr(iter, token.end());\
    BOOST_CHECK(r.is_ok());\
    if(r){BOOST_CHECK(r.unwrap() == expct);}\
    else{std::cerr << r.unwrap_err() << std::endl;}\
    BOOST_CHECK(iter == token.end());\
} /**/
