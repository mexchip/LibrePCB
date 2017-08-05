/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * http://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "device.h"
#include <librepcb/common/fileio/domdocument.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace library {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

Device::Device(const Uuid& uuid, const Version& version, const QString& author,
               const QString& name_en_US, const QString& description_en_US,
               const QString& keywords_en_US) throw (Exception) :
    LibraryElement(getShortElementName(), getLongElementName(), uuid, version, author,
                   name_en_US, description_en_US, keywords_en_US)
{
}

Device::Device(const FilePath& elementDirectory, bool readOnly) throw (Exception) :
    LibraryElement(elementDirectory, getShortElementName(), getLongElementName(), readOnly)
{
    DomElement& root = mLoadingXmlFileDocument->getRoot();

    // load attributes
    mComponentUuid = root.getFirstChild("component", true)->getText<Uuid>(true);
    mPackageUuid = root.getFirstChild("package", true)->getText<Uuid>(true);

    // load pad signal map
    foreach (const DomElement* node, root.getChilds("pad_signal_map")) {
        Uuid pad = node->getAttribute<Uuid>("pad", true);
        Uuid signal = node->getText<Uuid>(false);
        if (mPadSignalMap.contains(pad)) {
            throw RuntimeError(__FILE__, __LINE__,
                QString(tr("The pad \"%1\" exists multiple times in \"%2\"."))
                .arg(pad.toStr(), root.getDocFilePath().toNative()));
        }
        mPadSignalMap.insert(pad, signal);
    }

    cleanupAfterLoadingElementFromFile();
}

Device::~Device() noexcept
{
}

/*****************************************************************************************
 *  Pad-Signal-Map Methods
 ****************************************************************************************/

void Device::addPadSignalMapping(const Uuid& pad, const Uuid& signal) noexcept
{
    Q_ASSERT(!mPadSignalMap.contains(pad));
    mPadSignalMap.insert(pad, signal);
}

void Device::removePadSignalMapping(const Uuid& pad) noexcept
{
    Q_ASSERT(mPadSignalMap.contains(pad));
    mPadSignalMap.remove(pad);
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void Device::serialize(DomElement& root) const throw (Exception)
{
    LibraryElement::serialize(root);
    root.appendTextChild("component", mComponentUuid);
    root.appendTextChild("package", mPackageUuid);
    foreach (const Uuid& padUuid, mPadSignalMap.keys()) {
        DomElement* child = root.appendChild("pad_signal_map");
        child->setAttribute("pad", padUuid);
        child->setText(mPadSignalMap.value(padUuid));
    }
}

bool Device::checkAttributesValidity() const noexcept
{
    if (!LibraryElement::checkAttributesValidity())             return false;
    if (mComponentUuid.isNull())                                return false;
    if (mPackageUuid.isNull())                                  return false;
    foreach (const Uuid& padUuid, mPadSignalMap.keys()) {
        if (padUuid.isNull())                                   return false;
    }
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb
