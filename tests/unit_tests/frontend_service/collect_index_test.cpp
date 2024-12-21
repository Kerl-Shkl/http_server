#include "frontend_service.hpp"
#include <gtest/gtest.h>

TEST(FrontendService, CollectIndex)
{
    FrontendService front;
    front.collectIndex();
    EXPECT_EQ(front.resource_index["faq.html"].string(), "/home/kerl/work/C++/Server/html_css/faq.html");
    EXPECT_EQ(front.resource_index["home.html"].string(), "/home/kerl/work/C++/Server/html_css/home.html");
    EXPECT_EQ(front.resource_index["cv.html"].string(), "/home/kerl/work/C++/Server/html_css/CV/cv.html");
    EXPECT_EQ(front.resource_index["cv.css"].string(), "/home/kerl/work/C++/Server/html_css/CV/cv.css");
    EXPECT_EQ(front.resource_index["avatar_face.jpg"].string(),
              "/home/kerl/work/C++/Server/html_css/CV/avatar_face.jpg");
    EXPECT_EQ(front.resource_index.size(), 5);
}
