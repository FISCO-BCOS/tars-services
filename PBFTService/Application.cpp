#include "servant/Application.h"
#include "../Common/TarsUtils.h"
#include "PBFTServiceServer.h"

using namespace bcostars;

class PBFTServiceApp : public Application
{
public:
    virtual ~PBFTServiceApp() override{};

    virtual void initialize() override
    {
        addAllConfig(this);
        addServant<PBFTServiceServer>(
            ServerConfig::Application + "." + ServerConfig::ServerName + "." + PBFT_SERVANT_NAME);
    }

    virtual void destroyApp() override {}
};

int main(int argc, char* argv[])
{
    try
    {
        PBFTServiceApp app;
        app.main(argc, argv);
        app.waitForShutdown();

        return 0;
    }
    catch (std::exception& e)
    {
        cerr << "std::exception:" << e.what() << std::endl;
    }
    catch (...)
    {
        cerr << "unknown exception." << std::endl;
    }
    return -1;
}