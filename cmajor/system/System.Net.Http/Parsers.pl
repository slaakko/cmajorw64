namespace System.Net.Http
{
    grammar HttpHeaderGrammar
    {
        header : HttpHeader;
    }
    grammar HttpFieldValueGrammar
    {
        csvFieldValue(List<HttpFieldValue>* values);
    }
    grammar HttpGrammar
    {
        statusLine(HttpStatus* httpStatus);
        httpVersion : ustring;
        statusCode : ushort;
        reasonPhrase : ustring;
        charset;
        contentCoding;
        transferCoding;
        commaSeparatedFieldValue(List<HttpFieldValue>* values);
        generalFieldValue(List<HttpFieldValue>* values, var HttpFieldValue fieldValue);
        generalParameter(HttpFieldValue* fieldValue);
        chunkHeader(ulong* chunkSize, ChunkExtensionAdder* adder);
        chunkExtensions(ChunkExtensionAdder* adder);
        chunkExtName : ustring;
        chunkExtVal : ustring;
        transferExtension;
        parameter;
        attribute : ustring;
        value : ustring;
        mediaType;
        type;
        subtype;
        httpHeader : HttpHeader;
        fieldName : ustring;
        fieldValue : ustring;
        httpDate : DateTime;
        rfc1123Date : DateTime;
        rfc850Date : DateTime;
        asctimeDate(var Date monthDay) : DateTime;
        date1 : Date;
        date2 : Date;
        date3(var Month m, var sbyte d) : Date;
        Year : short;
        Year2 : short;
        Day : sbyte;
        Day1 : sbyte;
        wkday;
        weekday;
        month : Month;
        time : int;
        Hour : int;
        Min : int;
        Sec : int;
        deltaSeconds : int;
        httpToken : ustring;
        sep;
        comment;
        ctext : uchar;
        quotedString : ustring;
        qdtext : uchar;
        quotedPair : uchar;
        OCTET;
        CHAR : uchar;
        UPALPHA;
        LOALPHA;
        ALPHA;
        DIGIT;
        CTL;
        CR;
        LF;
        SP;
        HT;
        QUOTE;
        CRLF;
        LWS;
        TEXT;
        HEX;
    }
    grammar HttpStatusLineGrammar
    {
        status(HttpStatus* httpStatus);
    }
    grammar UriGrammar
    {
        UriReference(UriReference* uriReference);
        AbsoluteUri(UriReference* uriReference);
        RelativeUri(UriReference* uriReference);
        HierPart(UriReference* uriReference);
        OpaquePart;
        UriCNoSlash;
        NetPath(UriReference* uriReference);
        AbsPath;
        RelPath;
        RelSegment;
        Scheme;
        Authority(UriReference* uriReference);
        RegName;
        Server(UriReference* uriReference, var ustring userInfo);
        UserInfo;
        HostPort(UriReference* uriReference);
        Host;
        HostName(var ustring dl);
        DomainLabel;
        TopLabel;
        IPv4Address;
        Port : uint;
        Path;
        PathSegments;
        Segment;
        Param;
        PChar;
        Query;
        Fragment;
        UriC;
        Reserved;
        Unreserved;
        Mark;
        Escaped;
        Hex;
        AlphaNum;
        AlphaNumOrDash;
        Alpha;
        UpperAlpha;
        LowerAlpha;
        Digit;
    }
    grammar HttpChunkHeaderGrammar
    {
        header(ulong* chunkSize, ChunkExtensionAdder* adder);
    }
}
