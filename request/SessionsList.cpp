#include "SessionsList.hpp"

void    SessionsList::createUnicId()
{
    std::string alphaS = "abcdefghijklmnopqrstuvwxyz";
    std::string alphaU = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string digits = "0123456789";

    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            int random = rand();
            if (j == 0 && i != 0)
                id += '-';
            if (random % 3 == 1)
                id += alphaS[std::rand() % (alphaS.length() - 1)];
            else if (random % 3 == 2)
                id += alphaU[std::rand() % (alphaU.length() - 1)];
            else
                id += digits[std::rand() % (digits.length() - 1)];
        }
    }
}

void SessionsList::generateSession(Request &req)
{
    if (req.cookies.find("session_id") != req.cookies.end())
    {
        id = req.cookies["session_id"];
        if (!id.empty() && sessionLis.find(id) != sessionLis.end())
        {
            req.session = &sessionLis[id];
            req.session->lastLog = std::time(NULL);
            req.headers["set-cookie"] = "session_id=" + id + ";path=/";
            cout << "Khokho Bla3\n";
        }
        else
        { // I will improve the code to evit the repetition
            cout << "Khokho fla3\n";
            createUnicId();
            Sessions tmpSession;
            tmpSession.userName = id;
            tmpSession.lastLog = std::time(NULL);
            sessionLis[id] = tmpSession;

            req.session = &sessionLis[id];
            req.headers["set-cookie"] = "session_id=" + id + ";path=/";
        }
    }
    else
    {
        cout << "KO#KO#\n";
        createUnicId();
        Sessions tmpSession;
        tmpSession.userName = id;
        tmpSession.lastLog = std::time(NULL);
        sessionLis[id] = tmpSession;

        req.session = &sessionLis[id];
        req.headers["set-cookie"] = "session_id=" + id + ";path=/";
    }
    
}

void SessionsList::printSesions()
{

    for (std::map<std::string, Sessions>::iterator it = sessionLis.begin(); it != sessionLis.end(); ++it)
    {
        std::cout << it->first << " => " << it->second.lastLog << "\n";
    }

}

SessionsList::SessionsList()
{
    std::srand(std::time(NULL));
}

void    SessionsList::checkLimitSessions()
{
    time_t now = std::time(NULL);

    for (std::map<std::string, Sessions>::iterator it = sessionLis.begin(); it != sessionLis.end();)
    {
        if ( now - it->second.lastLog > 300 )
            sessionLis.erase(it++);
        else
            ++it;
    }
}