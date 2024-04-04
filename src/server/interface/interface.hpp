/**
 *
 *  @file         interface.hpp
 *
 *  @copyright    MIT License
 *
 *                Copyright (c) 2023 Sorin Tudose
 *
 *                Permission is hereby granted, free of charge, to any person obtaining a copy
 *                of this software and associated documentation files (the "Software"), to deal
 *                in the Software without restriction, including without limitation the rights
 *                to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *                copies of the Software, and to permit persons to whom the Software is
 *                furnished to do so, subject to the following conditions:
 *
 *                The above copyright notice and this permission notice shall be included in all
 *                copies or substantial portions of the Software.
 *
 *                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *                IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *                FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *                AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *                LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *                OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *                SOFTWARE.
 *
 *  @brief        This C++ header file provides core functionalities for user-related Server functionalities.
 *
 */

#ifndef __INTERFACE_HPP__
#define __INTERFACE_HPP__

#include <vector>
#include <string>
#include <stdlib.h>

enum : short
{
    NET_USERNAME_LENGHT = 32,
    NET_PASSWORD_LENGHT = 64
};

namespace net
{
    namespace interface
    {
        // Class describing a 'user' and it is part of the Server's interface namespace.
        class user
        {
        public:

        private:
            bool AUTH_STATUS; // This variable is used to check is the user is authentificated or not
            int SESSION_ID;   // This variable is used to load the correct user data. It is asigned the ID of the user who logs into their account.

        public:
            user();

            // This function retrieves the session ID
            int getSessionID(void) const noexcept;

            /**
             * @brief This function retrieves the authentification status.
             * @return Returns true if the user is signed in, false otherwise.
             */
            bool getAuthStatus(void) const noexcept;

            // This function is used to reset the variable that is used to check if the user is authentificated or not.
            void resetAuthStatus(void) noexcept;

            // This function is used to reset the website session ID.
            void resetSessionID(void) noexcept;


            /**
             *  @brief Add implementation
            */


            /**
             * @brief This function is used to dynamically build 'index.html' during runtime.
             *        When a new file is uploaded to the Server the page is rebuilt, generating a number of <tr> elements equal to the number of files sent.
             */
            void buildIndexHTML(void);

            ~user();
        };
    };
};

#endif // __INTERFACE_HPP__