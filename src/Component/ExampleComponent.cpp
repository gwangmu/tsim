#include <TSim/Pathway/Wire.h>
#include <TSim/Utility/Prototype.h>

#include <Component/ExampleComponent.h>
#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Message/ExampleMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


ExampleComponent::ExampleComponent (string iname, Component *parent)
    : Component ("ExampleComponent", iname, parent)
{
    // add child modules/components
    Module *datasource = new DataSourceModule ("datasource", this);
    AddChild (datasource);

    Module *datasink = new DataSinkModule ("datasink", this);
    AddChild (datasink);

    // create pathways
    Pathway::ConnectionAttr conattr;
    conattr.latency = 0;
    conattr.bitwidth = 32;
    Wire *d2dwire = new Wire (this, conattr, Prototype<ExampleMessage>::Get());

    // connect modules
    datasource->Connect ("dataout", d2dwire->GetEndpoint (Endpoint::LHS));
    datasink->Connect ("datain", d2dwire->GetEndpoint (Endpoint::RHS));
}