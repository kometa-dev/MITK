/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkStringPropertySerializer_h_included
#define mitkStringPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkStringProperty.h"

#include "StringUtilities.h"

namespace mitk
{

class StringPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( StringPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const StringProperty* prop = dynamic_cast<const StringProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("string");
        auto str = prop->GetValue();
        if (Utilities::isValidUtf8(str)) {
          element->SetAttribute("value", str);
        } else {
          element->SetAttribute("value", Utilities::convertLocalToUTF8(str));
        }
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;
      const char* s( element->Attribute("value") );
      return StringProperty::New( std::string(s?s:"") ).GetPointer();
    }

  protected:

    StringPropertySerializer() {}
    virtual ~StringPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringPropertySerializer);

#endif

