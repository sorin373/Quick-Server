#include "interface.hpp"

#include "../serverUtils.hpp"
#include "../global.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>

using namespace net;
using namespace net::interface;

user::user()
{
    this->AUTH_STATUS = true; // if there is a auth sys implemented set this to false
    this->SESSION_ID = -1;
}

int user::getSessionID(void) const noexcept
{
    return SESSION_ID;
}

bool user::getAuthStatus(void) const noexcept
{
    return AUTH_STATUS;
}

void user::resetAuthStatus(void) noexcept
{
    this->AUTH_STATUS = false;
}

void user::resetSessionID(void) noexcept
{
    this->SESSION_ID = -1;
}

void user::buildIndexHTML(void)
{
    std::ofstream index_html(INDEX_HTML_PATH);

    if (!index_html.is_open())
    {
        std::cerr << std::setw(5) << " "
                  << "Error: Failed to open file: " << INDEX_HTML_PATH << "\n";
        return;
    }

    char firstHTML[] = R"(<!DOCTYPE html>
                        <html lang="en">

                        <head>
                            <meta charset="UTF-8"/>
                            <meta name="viewport" content="width=device-width, initial-scale=1.0"/>

                            <title>http-Server</title>
                            
                            <link crossorigin="anonymous" href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css"
                                integrity="sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3" rel="stylesheet"/>
                            <script crossorigin="anonymous" src="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js"
                                integrity="sha384-ka7Sk0Gln4gmtz2MlQnikT1wXgYsOg+OMhuP+IlRH9sENBO0LRn5q+8nbTov4+1p"></script>

                            <link rel="icon" href="static/assets/logo.png"/>
                            <link href="static/stylesheet/index.css" rel="stylesheet"/>

                            <style>
                                a {
                                    text-decoration: none;
                                    font-size: 15pt;
                                    padding: 1px 6px;
                                    color: rgb(66, 66, 66) !important;
                                }

                                a:hover {
                                    color: #005ed8 !important;
                                }
                            </style>
                        </head>

                        <body>
                            <div class="main-container">
                                <div class="title-container">
                                    <p>http-Server</p>
                                </div>
                                <div class="container">)";

    index_html << firstHTML;

    char HTMLclose[] = R"(
        </div>

        </div>

        <div class="footer">
            <p>Copyright (c) 2023 Sorin Tudose | </p>
            <a href="https://github.com/sorin373/HTTP-Server">GitHub</a>
        </div>

        </body>
        </html>
     )";

    index_html << HTMLclose;

    index_html.close();
}

user::~user(){}