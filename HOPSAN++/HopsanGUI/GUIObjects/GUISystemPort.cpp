//$Id$

#include "GUISystemPort.h"
#include "GUISystem.h"
#include "../GUIPort.h"
#include "../loadObjects.h"

//! @todo rename GUISystemPort to ContainerPort
GUISystemPort::GUISystemPort(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, GUIContainerObject *pParentContainer, selectionStatus startSelected, graphicsType gfxType)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    qDebug() << "GUISystemPort: ,,,,,,,,,,,,,,setting parent to: " << pParentContainer;
    mIsSystemPort = (pParentContainer->type() == GUISYSTEM); //determine if I am a system port
    this->mHmfTagName = HMF_SYSTEMPORTTAG;
    //Sets the ports
    createPorts();
    refreshDisplayName();
}

GUISystemPort::~GUISystemPort()
{
    qDebug() << "GuiSystemPort destructor: " << this->getName();
    if (mIsSystemPort)
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->deleteSystemPort(this->getName());
    }
    else
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->unReserveUniqueName(this->getName());
    }
}

//! @brief Help function to create ports in the SystemPort Object when it is created
void GUISystemPort::createPorts()
{
    //A system port only contains one port, which should be first in the map, ignore any others (should not be any more)
    PortAppearanceMapT::iterator i = mGUIModelObjectAppearance.getPortAppearanceMap().begin();

    //Check if a systemport name is given in appearance data (for example if the systemport is loaded from file)
    //In that case override the default port name with this name
    QString desiredportname;
    if (mGUIModelObjectAppearance.getName().isEmpty())
    {
        desiredportname = i.key();
    }
    else
    {
        desiredportname = mGUIModelObjectAppearance.getName();
    }

    qreal x = i.value().x;
    qreal y = i.value().y;

    //! @todo should make this function select a systemport icon not undefined
    i.value().selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)


    if (mIsSystemPort)
    {
        qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Adding systemport with name: " << desiredportname;
        mGUIModelObjectAppearance.setName(mpParentContainerObject->getCoreSystemAccessPtr()->addSystemPort(desiredportname));
        qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,resulting in name from core: " << mGUIModelObjectAppearance.getName();
    }
    else
    {
        qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Adding groupport with desired name: " << desiredportname;
        mGUIModelObjectAppearance.setName(mpParentContainerObject->getCoreSystemAccessPtr()->reserveUniqueName(desiredportname));
    }

    mpGuiPort = new GUIPort(mGUIModelObjectAppearance.getName(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this);
    mPortListPtrs.append(mpGuiPort);

}


//! Returns a string with the GUIObject type.
//! @todo maybe not hardcoded string
QString GUISystemPort::getTypeName()
{
    if (mIsSystemPort)
    {
        return "SystemPort";
    }
    else
    {
        //! @todo return something usefulould make sure that the gui can register these guispecific names in core to avoid creating objects with these type names
        //! @todo we sh
        return "GUIGroupPort";
    }
}

//! Set the name of a system port
void GUISystemPort::setName(QString newName, renameRestrictions renameSettings)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        if (renameSettings == CORERENAMEONLY)
        {
            if (mIsSystemPort)
            {
                //Set name in core component, Also set the current name to the resulting one (might have been changed)
                mGUIModelObjectAppearance.setName(mpParentContainerObject->getCoreSystemAccessPtr()->renameSystemPort(oldName, newName));
            }
            else
            {
                //! @todo we need to make sure we can rename with unique name in gui, only for the gui only specifik stuff
                mGUIModelObjectAppearance.setName(newName);
            }

            refreshDisplayName();
            mpGuiPort->setDisplayName(mGUIModelObjectAppearance.getName()); //change the actual gui port name
        }
        else
        {
            //! @todo we need to make sure we can rename with unique name in gui, only for the gui only specifik stuff
            //Rename
            mpParentContainerObject->renameGUIModelObject(oldName, newName);
        }
    }
}


int GUISystemPort::type() const
{
    return Type;
}
