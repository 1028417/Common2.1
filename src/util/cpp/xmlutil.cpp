
#include "util.h"

#include "../../3rd/tinyxml/tinyxml.h"

static bool _findChild(tagXmlElementInfo& elementInfo, const list<string>& lstChildName
                , list<string>::const_iterator& itrChildName, CB_XmlGetChild cb)
{
    if (itrChildName == lstChildName.end())
    {
        return true;
    }

    for (tagXmlElementInfo& childElementInfo : elementInfo.vecChildInfo)
    {
        if (childElementInfo.strName == *itrChildName)
        {
            ++itrChildName;
            if (itrChildName != lstChildName.end())
            {
                if (!_findChild(childElementInfo, lstChildName, itrChildName, cb))
                {
                    return false;
                }
            }
            else
            {
                if (!cb(childElementInfo))
                {
                    return false;
                }
            }
            --itrChildName;
        }
    }

    return true;
}

void tagXmlElementInfo::getChild(const list<string>& lstChildName, CB_XmlGetChild cb)
{
    auto itrChildName = lstChildName.begin();
    (void)_findChild(*this, lstChildName, itrChildName, cb);
}

void tagXmlElementInfo::getChildAttr(const list<string>& lstChildName, const string& strAttrName, CB_XmlGetChildAttr cb)
{
    getChild(lstChildName, [&](tagXmlElementInfo& childInfo){
        cauto& itr = childInfo.mapAttr.find(strAttrName);
        if (itr != childInfo.mapAttr.end())
        {
            if (!cb(childInfo, itr->second))
            {
                return false;
            }
        }

        return true;
    });
}

static void _loadElement(const TiXmlElement& element, tagXmlElementInfo& elementInfo)
{
    elementInfo.strName = element.ValueStr();
    cauto pText = element.GetText();
    if (pText)
    {
        elementInfo.strText = pText;
    }

    for (auto pAttr = element.FirstAttribute(); pAttr; pAttr = pAttr->Next())
    {
        elementInfo.mapAttr[pAttr->NameTStr()] = pAttr->ValueStr();
    }

    for (auto pChild = element.FirstChildElement(); pChild; pChild = pChild->NextSiblingElement())
    {
        elementInfo.vecChildInfo.push_back(tagXmlElementInfo());
        _loadElement(*pChild, elementInfo.vecChildInfo.back());
    }
}

static bool _loadXml(const string& strFile, bool bUtf8, bool bHtml, tagXmlElementInfo& rootElementInfo)
{
	TiXmlDocument doc(bHtml);
	if (!doc.LoadFile(strFile, bUtf8? TIXML_ENCODING_UTF8:TIXML_DEFAULT_ENCODING))
	{
		return false;
	}

    cauto pRoot = doc.RootElement();
    if (pRoot)
    {
        _loadElement(*pRoot, rootElementInfo);
    }

	return true;
}

bool xmlutil::loadXml(const string& strFile, tagXmlElementInfo& rootElementInfo, bool bUtf8)
{
    return _loadXml(strFile, bUtf8, false, rootElementInfo);
}

bool xmlutil::loadHtml(const string& strFile, tagXmlElementInfo& rootElementInfo, bool bUtf8)
{
    return _loadXml(strFile, bUtf8, true, rootElementInfo);
}
