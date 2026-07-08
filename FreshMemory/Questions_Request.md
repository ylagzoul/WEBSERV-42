    0 Still not readed
    
    ++ read each part of request Http .
    
    -+ Youssed must learn what is the RFC.

    ++ I must understand the url in RFC. Uniform resource locator

    ++ Reserved and unreserved caracters.

    ++ Undersranding why we use chunked in tran-enc. => when the client dosn't know the size of data that he reads . he reads a peice of data,  count it to aquire the length , convert it to hex number then send it with the data to the server (why Hex num ? => the RFC is like this. + it print small instead of decimal it print long ).  

    ++ What \r do ?

    ++ \r\n Name ? => CRLF

    ++ char hexToChar(char a, char b) => I changed it
        {
            int high = hexValue(a);
            int low  = hexValue(b);

            return ((high << 4) | low); ==> (unsigned char)res > 127
        }
    ++ should I handle cookie with ';'
    
    ++ 
        std::remove(str.begin(), str.end(), '\r');
        std::remove(str.begin(), str.end(), '\n');
            ----> test and then fix


    ++  std::string tmp = req.fullLine.substr(0, endNum + 2);
        if (!checkDigits(req, tmp))
            return(close(req.bodyFd), returnedBadError(req));
        endTxt = req.fullLine.find("\r\n", endNum + 2); // 0/r/nfile/r/n   EndNum = 1  EndTxt= 4
        cout << "<" << req.fullLine << ">" << "\n";
        cout <<  endNum << "--" << endTxt << std::endl;


    ++    //;path=/ need more under => Only send this cookie for URLs whose path starts with /

    ++  => for save the itertor and don't lose it
        for (std::map<std::string, Sessions>::iterator it  sessionLis.begin(); it != sessionLis.end();)
    {
        time_t now = std::time(NULL);
        if ( now - it->second.lastLog > 30 )
        {
            sessionLis.erase(it++);
            // cout << "DKHELT\n";
        }
        else
            it++;//
    }

    
    ++ what is HTTP? what is requests ? 
    
    + different from 1.0 and 1.1
    
    ++ \r\n name and brief explanation about it 
    
    ++ brief explanation about status codes generally
    
    + uri vs url - url types
    
    ++ rfc reseved and unreserved chars
    
    ++ what is a static website ?
    
    + sockets , what is bind, accept, pool, what listen do , tcp basics
    
    ++ what is localhost ? => loopback address  --> the network traffic "loops back" to the same machine
    
    + why we need cgi ? and which type of cgi we used
    
    ++ what is cookies and sesion managment why we need them cookies type
    
    + what is locations, directive, block server
    
    + I/O blocking and I/O non blocking 
    
    + multiple cookies how's that gonna help?

---------------------------- may simlar to tcp layers or osi
? headers that describe the transport (Content-Length, Connection 
? headers that carry application data (Set-Cookie, Content-Type, custom headers)
----------------------------