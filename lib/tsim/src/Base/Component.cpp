#include <TSim/Base/Component.h>

#include <TSim/Simulation/Simulator.h>
#include <TSim/Module/Module.h>
#include <TSim/Device/Device.h>
#include <TSim/Pathway/Pathway.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <string>
#include <vector>
#include <map>

using namespace std;


Component::Component (const char *clsname, string name, Component *parent)
    : Metadata (clsname, name)
{
    this->parent = parent;
    if (parent)
        parent->AddChild (this);

    this->clockname = "";
}


string Component::GetClock ()
{ 
    if (!clockname.empty ())
        return clockname;
    else if (!parent)
        return "";
    else
        return parent->GetClock();
}

string Component::GetFullName ()
{
    return (string(GetClassName()) + " " + GetFullNameWOClass());
}

string Component::GetFullNameWOClass ()
{
    string familyname = "";
    if (parent)
        familyname = parent->GetFullNameWOClass ();

    if (familyname.empty ())
        return GetInstanceName();
    else
        return (familyname + "::" + GetInstanceName());
}

uint32_t Component::GetNumChildModules ()
{
    uint32_t nmods = 0;
    for (Component *child : children)
        nmods += child->GetNumChildModules ();

    return nmods;
}


Module* Component::GetModule (string name)
{
    Module *tar = nullptr;
    for (Component *child : children)
    {
        if ((tar = child->GetModule (name)))
            break;
    }

    return tar;
}

Component::CycleClass<double> Component::GetAggregateCycleClass ()
{
    CycleClass<double> cclass;

    for (Component *child : children)
    {
        CycleClass<double> child_cclass;
        child_cclass = child->GetAggregateCycleClass ();
        
        cclass.active += child_cclass.active;
        cclass.idle += child_cclass.idle;
    }

    return cclass;
}

Component::EventCount<double> Component::GetAggregateEventCount ()
{
    EventCount<double> ecount;

    for (Component *child : children)
    {
        EventCount<double> child_ecount;
        child_ecount = child->GetAggregateEventCount ();
        
        ecount.stalled += child_ecount.stalled;
        // NOTE: ecount.oport_full is non-aggregatable
    }

    return ecount;
}

double Component::GetAggregateConsumedEnergy ()
{
    double energy = 0;

    for (Component *child : children)
    {
        double cenergy = child->GetAggregateConsumedEnergy ();
        if (cenergy != -1)
            energy += cenergy;
        else
            return -1;
    }
    for (Pathway *pathway : pathways)
    {
        double penergy = pathway->GetConsumedEnergy ();
        if (penergy != -1)
            energy += penergy;
        else
            return -1;
    }

    return energy;
}


string Component::GetGraphVizBody (uint32_t level)
{
    string body = "";

#define ADDBULK(str) body += (str);
#define ADDLINE(str) body += (string(level, '\t') + str + string("\n"));

    operation ("node declaration")
    {
        ADDLINE (string("// ") + GetFullName());
        ADDLINE ("subgraph " + string("cluster_") + GetInstanceName() + " {");
        ADDLINE ("\tlabel = \"" + GetName() + "\"");

        for (Component *child : children)
            ADDBULK (child->GetGraphVizBody (level + 1));

        for (Pathway *pathway : pathways)
            if (pathway->GetNumLHS() > 1 || pathway->GetNumRHS() > 1)
            {
                string label = "";
                label = pathway->GetMsgPrototype()->GetClassName() + string ("(") + 
                        to_string(pathway->GetBitWidth()) + string (")");

                ADDLINE ("p" + to_string((uint64_t)pathway) + " [xlabel=\"" + label + "\", shape=point]");
            }

        ADDLINE ("}");
    }

    operation ("connection")
    {
        for (Pathway *pathway : pathways)
        {
            vector<string> lhss;
            vector<string> rhss;
            
            for (auto i = 0; i < pathway->GetNumLHS(); i++)
            {
                Endpoint &endpt = pathway->GetLHS(i);
                string lhsname = "";

                if (endpt.GetConnectedModule ())
                    lhsname += endpt.GetConnectedModule()->GetInstanceName() + ":";
                else if (endpt.GetConnectedDevice ())
                    lhsname += endpt.GetConnectedDevice()->GetInstanceName() + ":";
                else
                    SYSTEM_ERROR ("bogus component type");

                lhsname += endpt.GetConnectedPortName();

                lhss.push_back (lhsname);
            }

            for (auto i = 0; i < pathway->GetNumRHS(); i++)
            {
                Endpoint &endpt = pathway->GetRHS(i);
                string rhsname = "";

                if (endpt.GetConnectedModule ())
                    rhsname += endpt.GetConnectedModule()->GetInstanceName() + ":";
                else if (endpt.GetConnectedDevice ())
                    rhsname += endpt.GetConnectedDevice()->GetInstanceName() + ":";
                else
                    SYSTEM_ERROR ("bogus component type");

                rhsname += endpt.GetConnectedPortName();

                rhss.push_back (rhsname);
            }

            if (lhss.size() > 0 && rhss.size() > 0)
            {
                string label = "";
                label = pathway->GetMsgPrototype()->GetClassName() + string ("(") + 
                        to_string(pathway->GetBitWidth()) + string (")");

                if (lhss.size() == 1 && rhss.size() == 1)
                {
                    ADDLINE (lhss[0] + " -> " + rhss[0] + " [label=\"" + label + "\"];");
                }
                else
                {
                    string point = "p" + to_string((uint64_t)pathway);

                    for (auto i = 0; i < lhss.size(); i++)
                        ADDLINE (lhss[i] + " -> " + point);
                    for (auto i = 0; i < rhss.size(); i++)
                        ADDLINE (point + " -> " + rhss[i] + " [arrowtail=inv, dir=both];");
                }
            }
        }
    }    

#undef ADDLINE
#undef ADDBULK
    
    return body;
}


bool Component::AddChildPathway (Pathway *pathway, PERMIT(Pathway))
{
    for (Pathway *cpathway : pathways)
    {
        if (cpathway == pathway)
        {
            DESIGN_WARNING ("already has '%s' as a child. ignoring",
                    GetFullName().c_str(), pathway->GetName().c_str());
            return true;
        }
    }

    pathways.push_back (pathway);

    return true;
}


IssueCount Component::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (GetClock().empty ())
    {
        DESIGN_ERROR ("no clock assigned", GetFullName().c_str());
        icount.error++;
    }

    for (Component *comp : children)
    {
        IssueCount subicount = comp->Validate (TRANSFER_KEY(Simulator));
        icount.error += subicount.error;
        icount.warning += subicount.warning;
    }

    for (Pathway *pathway : pathways)
    {
        IssueCount subicount = pathway->Validate (TRANSFER_KEY(Simulator));
        icount.error += subicount.error;
        icount.warning += subicount.warning;
    }

    return icount;
}


bool Component::SetClock (string clockname)
{
    if (!this->clockname.empty ())
        DESIGN_WARNING ("changing clock '%s' to '%s'",
                GetFullName().c_str(), this->clockname.c_str(), clockname.c_str());

    this->clockname = clockname;
    return true;
}

bool Component::Connect (string portname, Endpoint *endpt)
{
    if (!portname2tag.count (portname))
    {
        DESIGN_ERROR ("non-existing port '%s'",
                GetFullName().c_str(), portname.c_str());
        return false;
    }

    if (!portname2tag[portname].child) {
        SYSTEM_ERROR ("null exported port (who: %s, portname: %s)",
                GetFullName().c_str(), portname2tag[portname].portname.c_str());
        return false;
    }

    return portname2tag[portname].child->Connect (
            portname2tag[portname].portname, 
            endpt);
}

bool Component::ExportPort (string eportname, Component *child, string cportname)
{
    if (child == nullptr)
    {
        DESIGN_ERROR ("cannot export null child's port",
                GetFullName().c_str());
        return false;
    }

    if (portname2tag.count (eportname))
    {
        DESIGN_WARNING ("port '%s' already exist. overwriting",
                GetFullName().c_str(), eportname.c_str());
    }

    PortExportTag tag;
    tag.child = child;
    tag.portname = cportname;

    portname2tag[eportname] = tag;

    return true;
}

bool Component::AddChild (Component *child)
{
    if (child == nullptr)
    {
        DESIGN_ERROR ("cannot add null child", GetFullName().c_str());
        return false;
    }

    if (IsAncestor (child) || IsDescendant (child))
    {
        DESIGN_ERROR ("%s is already in the hierarchy",
                GetFullName().c_str(), child->GetFullName().c_str());
        return false;
    }

    children.push_back (child);

    return true;
}
