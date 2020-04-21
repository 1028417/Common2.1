
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

void tagXmlElementInfo::getChild(const list<string>& lstChildName, const string& strAttrName
              , const string& strAttrValue, CB_XmlGetChild cb)
{
    getChild(lstChildName, [&](tagXmlElementInfo& elementInfo){
        cauto itr = elementInfo.mapAttr.find(strAttrName);
        if (itr != elementInfo.mapAttr.end())
        {
            if (itr->second == strAttrValue)
            {
                if (!cb(elementInfo))
                {
                    return false;
                }
            }
        }

        return true;
    });
}

void tagXmlElementInfo::getChildAttr(const list<string>& lstChildName, const string& strAttrName, CB_XmlGetChildAttr cb)
{
    getChild(lstChildName, [&](tagXmlElementInfo& childInfo){
        cauto itr = childInfo.mapAttr.find(strAttrName);
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

void tagXmlElementInfo::getChildAttr(const list<string>& lstChildName, const string& strAttrName, list<string>& lstAttrValue)
{
    getChildAttr(lstChildName, strAttrName, [&](tagXmlElementInfo&, const string& strAttrValue){
        lstAttrValue.push_back(strAttrValue);

        return true;
    });
}

static void _loadElement(const TiXmlElement& element, tagXmlElementInfo& elementInfo)
{
    elementInfo.strName = element.ValueStr();
    auto pText = element.GetText();
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
        elementInfo.vecChildInfo.emplace_back();
        _loadElement(*pChild, elementInfo.vecChildInfo.back());
    }
}

bool xmlutil::loadXml(char* buf, size_t length, bool bUtf8, bool bHtml, tagXmlElementInfo& rootElementInfo)
{
	TiXmlDocument doc(bHtml);
	if (!doc.Load(buf, length, bUtf8 ? TIXML_ENCODING_UTF8 : TIXML_DEFAULT_ENCODING))
	{
		return false;
	}

	auto pRoot = doc.RootElement();
	if (pRoot)
	{
		_loadElement(*pRoot, rootElementInfo);
	}

	return true;
}

bool xmlutil::loadXml(Instream& ins, bool bHtml, tagXmlElementInfo& rootElementInfo)
{
	string strText;
	CTxtReader TxtReader;
    auto eHeadType = TxtReader.read(ins, strText);

	bool bUtf8 = E_TxtHeadType::THT_UTF8Bom == eHeadType || strutil::checkUtf8(strText);

	return loadXml((char *)strText.c_str(), strText.size(), bUtf8, bHtml, rootElementInfo);
}
