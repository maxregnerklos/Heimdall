from PyQt5.QtCore import QFile, QIODevice, QXmlStreamReader, QXmlStreamWriter
from Alerts import Alerts

class DeviceInfo:
    def __init__(self):
        self.manufacturer = ""
        self.product = ""
        self.name = ""

    def parse_xml(self, xml):
        found_manufacturer = False
        found_product = False
        found_name = False

        while not xml.atEnd():
            next_token = xml.readNext()

            if next_token == QXmlStreamReader.StartElement:
                if xml.name() == "manufacturer":
                    if found_manufacturer:
                        Alerts.display_error("Found multiple <manufacturer> elements in <device>.")
                        return False

                    found_manufacturer = True
                    self.manufacturer = xml.readElementText()
                elif xml.name() == "product":
                    if found_product:
                        Alerts.display_error("Found multiple <product> elements in <device>.")
                        return False

                    found_product = True
                    self.product = xml.readElementText()
                elif xml.name() == "name":
                    if found_name:
                        Alerts.display_error("Found multiple <name> elements in <device>.")
                        return False

                    found_name = True
                    self.name = xml.readElementText()
                else:
                    Alerts.display_error(f"<{xml.name()}> is not a valid child of <device>.")
                    return False
            elif next_token == QXmlStreamReader.EndElement:
                if xml.name() == "device":
                    if found_manufacturer and found_product and found_name:
                        return True
                    else:
                        Alerts.display_error("Required elements are missing from <device>.")
                        return False
            else:
                if not (next_token == QXmlStreamReader.Characters and xml.isWhitespace()):
                    Alerts.display_error("Unexpected token found in <device>.")
                    return False

        return False

    def write_xml(self, xml):
        xml.writeStartElement("device")

        xml.writeStartElement("manufacturer")
        xml.writeCharacters(self.manufacturer)
        xml.writeEndElement()

        xml.writeStartElement("product")
        xml.writeCharacters(self.product)
        xml.writeEndElement()

        xml.writeStartElement("name")
        xml.writeCharacters(self.name)
        xml.writeEndElement()

        xml.writeEndElement()


class PlatformInfo:
    def __init__(self):
        self.name = ""
        self.version = ""

    def parse_xml(self, xml):
        found_name = False
        found_version = False

        while not xml.atEnd():
            next_token = xml.readNext()

            if next_token == QXmlStreamReader.StartElement:
                if xml.name() == "name":
                    if found_name:
                        Alerts.display_error("Found multiple <name> elements in <platform>.")
                        return False

                    found_name = True
                    self.name = xml.readElementText()
                elif xml.name() == "version":
                    if found_version:
                        Alerts.display_error("Found multiple <version> elements in <platform>.")
                        return False

                    found_version = True
                    self.version = xml.readElementText()
                else:
                    Alerts.display_error(f"<{xml.name()}> is not a valid child of <platform>.")
                    return False
            elif next_token == QXmlStreamReader.EndElement:
                if xml.name() == "platform":
                    if found_name and found_version:
                        return True
                    else:
                        Alerts.display_error("Required elements are missing from <platform>.")
                        return False
            else:
                if not (next_token == QXmlStreamReader.Characters and xml.isWhitespace()):
                    Alerts.display_error("Unexpected token found in <platform>.")
                    return False

        return False

    def write_xml(self, xml):
        xml.writeStartElement("platform")

        xml.writeStartElement("name")
        xml.writeCharacters(self.name)
        xml.writeEndElement()

        xml.writeStartElement("version")
        xml.writeCharacters(self.version)
        xml.writeEndElement()

        xml.writeEndElement()


class FileInfo:
    def __init__(self):
        self.partition_id = 0
        self.filename = ""

    def parse_xml(self, xml):
        found_id = False
        found_filename = False

        while not xml.atEnd():
            next_token = xml.readNext()

            if next_token == QXmlStreamReader.StartElement:
                if xml.name() == "id":
                    if found_id:
                        Alerts.display_error("Found multiple <id> elements in <file>.")
                        return False

                    found_id = True
                    self.partition_id = int(xml.readElementText())
                elif xml.name() == "filename":
                    if found_filename:
                        Alerts.display_error("Found multiple <filename> elements in <file>.")
                        return False

                    found_filename = True
                    self.filename = xml.readElementText()
                else:
                    Alerts.display_error(f"<{xml.name()}> is not a valid child of <file>.")
                    return False
            elif next_token == QXmlStreamReader.EndElement:
                if xml.name() == "file":
                    if found_id and found_filename:
                        return True
                    else:
                        Alerts.display_error("Required elements are missing from <file>.")
                        return False
            else:
                if not (next_token == QXmlStreamReader.Characters and xml.isWhitespace()):
                    Alerts.display_error("Unexpected token found in <file>.")
                    return False

        return False

    def write_xml(self, xml, filename):
        xml.writeStartElement("file")

        xml.writeStartElement("id")
        xml.writeCharacters(str(self.partition_id))
        xml.writeEndElement()

        xml.writeStartElement("filename")
        xml.writeCharacters(filename)
        xml.writeEndElement()

        xml.writeEndElement()


class FirmwareInfo:
    kVersion = 1

    def __init__(self):
        self.name = ""
        self.version = ""
        self.platform_info = PlatformInfo()
        self.developers = []
        self.url = ""
        self.donate_url = ""
        self.device_infos = []
        self.pit_filename = ""
        self.repartition = False
        self.no_reboot = False
        self.file_infos = []

    def parse_xml(self, xml):
        self.clear()

        found_name = False
        found_version = False
        found_platform = False
        found_developers = False
        found_url = False
        found_donate_url = False
        found_devices = False
        found_pit = False
        found_repartition = False
        found_no_reboot = False
        found_files = False

        if not xml.readNextStartElement():
            Alerts.display_error("Failed to find <firmware> element.")
            return False

        if xml.name() != "firmware":
            Alerts.display_error(f"Expected <firmware> element but found <{xml.name()}>.")
            return False

        format_version_string = xml.attributes().value("version")

        if format_version_string.isEmpty():
            Alerts.display_error("<firmware> is missing the version attribute.")
            return False

        parsed_version, format_version = False, format_version_string.toInt(parsed_version)
        if not parsed_version:
            Alerts.display_error("<firmware> contains a malformed version.")
            return False

        if format_version > FirmwareInfo.kVersion:
            Alerts.display_error("Package is for a newer version of Heimdall Frontend.\nPlease download the latest version of Heimdall Frontend.")
            return False

        while not xml.atEnd():
            next_token = xml.readNext()

            if next_token == QXmlStreamReader.StartElement:
                if xml.name() == "name":
                    if found_name:
                        Alerts.display_error("Found multiple <name> elements in <firmware>.")
                        return False

                    found_name = True
                    self.name = xml.readElementText()
                elif xml.name() == "version":
                    if found_version:
                        Alerts.display_error("Found multiple <version> elements in <firmware>.")
                        return False

                    found_version = True
                    self.version = xml.readElementText()
                elif xml.name() == "platform":
                    if found_platform:
                        Alerts.display_error("Found multiple <platform> elements in <firmware>.")
                        return False

                    found_platform = True
                    if not self.platform_info.parse_xml(xml):
                        return False
                elif xml.name() == "developers":
                    if found_developers:
                        Alerts.display_error("Found multiple <developers> elements in <firmware>.")
                        return False

                    found_developers = True
                    while not xml.atEnd():
                        next_token = xml.readNext()

                        if next_token == QXmlStreamReader.StartElement:
                            if xml.name() == "name":
                                self.developers.append(xml.readElementText())
                            else:
                                Alerts.display_error(f"<{xml.name()}> is not a valid child of <developers>.")
                                return False
                        elif next_token == QXmlStreamReader.EndElement:
                            if xml.name() == "developers":
                                break
                        else:
                            if not (next_token == QXmlStreamReader.Characters and xml.isWhitespace()):
                                Alerts.display_error("Unexpected token found in <developers>.")
                                return False
                elif xml.name() == "url":
                    if found_url:
                        Alerts.display_error("Found multiple <url> elements in <firmware>.")
                        return False

                    found_url = True
                    self.url = xml.readElementText()
                elif xml.name() == "donateurl":
                    if found_donate_url:
                        Alerts.display_error("Found multiple <donateurl> elements in <firmware>.")
                        return False

                    found_donate_url = True
                    self.donate_url = xml.readElementText()
                elif xml.name() == "devices":
                    if found_devices:
                        Alerts.display_error("Found multiple <devices> elements in <firmware>.")
                        return False

                    found_devices = True
                    while not xml.atEnd():
                        next_token = xml.readNext()

                        if next_token == QXmlStreamReader.StartElement:
                            if xml.name() == "device":
                                device_info = DeviceInfo()
                                if not device_info.parse_xml(xml):
                                    return False
                                self.device_infos.append(device_info)
                            else:
                                Alerts.display_error(f"<{xml.name()}> is not a valid child of <devices>.")
                                return False
                        elif next_token == QXmlStreamReader.EndElement:
                            if xml.name() == "devices":
                                break
                        else:
                            if not (next_token == QXmlStreamReader.Characters and xml.isWhitespace()):
                                Alerts.display_error("Unexpected token found in <devices>.")
                                return False
                elif xml.name() == "pit":
                    if found_pit:
                        Alerts.display_error("Found multiple <pit> elements in <firmware>.")
                        return False

                    found_pit = True
                    self.pit_filename = xml.readElementText()
                elif xml.name() == "repartition":
                    if found_repartition:
                        Alerts.display_error("Found multiple <repartition> elements in <firmware>.")
                        return False

                    found_repartition = True
                    self.repartition = (xml.readElementText().toInt() != 0)
                elif xml.name() == "noreboot":
                    if found_no_reboot:
                        Alerts.display_error("Found multiple <noreboot> elements in <firmware>.")
                        return False

                    found_no_reboot = True
                    self.no_reboot = (xml.readElementText().toInt() != 0)
                elif xml.name() == "files":
                    if found_files:
                        Alerts.display_error("Found multiple <files> elements in <firmware>.")
                        return False

                    found_files = True
                    while not xml.atEnd():
                        next_token = xml.readNext()

                        if next_token == QXmlStreamReader.StartElement:
                            if xml.name() == "file":
                                file_info = FileInfo()
                                if not file_info.parse_xml(xml):
                                    return False
                                self.file_infos.append(file_info)
                            else:
                                Alerts.display_error(f"<{xml.name()}> is not a valid child of <files>.")
                                return False
                        elif next_token == QXmlStreamReader.EndElement:
                            if xml.name() == "files":
                                break
                        else:
                            if not (next_token == QXmlStreamReader.Characters and xml.isWhitespace()):
                                Alerts.display_error("Unexpected token found in <devices>.")
                                return False
                else:
                    Alerts.display_error(f"<{xml.name()}> is not a valid child of <firmware>.")
                    return False
            elif next_token == QXmlStreamReader.EndElement:
                if xml.name() == "firmware":
                    if not (found_name and found_version and found_platform and found_developers and found_devices and found_pit and found_repartition and found_no_reboot and found_files):
                        Alerts.display_error("Required elements are missing from <firmware>.")
                        return False
                    else:
                        break
            else:
                if not (next_token == QXmlStreamReader.Characters and xml.isWhitespace()):
                    Alerts.display_error("Unexpected token found in <firmware>.")
                    return False

        xml.readNext()
        if not xml.atEnd():
            Alerts.display_error("Found data after </firmware>.")
            return False

        return True

    def write_xml(self, xml):
        xml.writeStartDocument()
        xml.writeStartElement("firmware")
        xml.writeAttribute("version", str(FirmwareInfo.kVersion))

        xml.writeStartElement("name")
        xml.writeCharacters(self.name)
        xml.writeEndElement()

        xml.writeStartElement("version")
        xml.writeCharacters(self.version)
        xml.writeEndElement()

        self.platform_info.write_xml(xml)

        xml.writeStartElement("developers")
        for dev in self.developers:
            xml.writeStartElement("name")
            xml.writeCharacters(dev)
            xml.writeEndElement()
        xml.writeEndElement()

        if self.url:
            xml.writeStartElement("url")
            xml.writeCharacters(self.url)
            xml.writeEndElement()

        if self.donate_url:
            xml.writeStartElement("donateurl")
            xml.writeCharacters(self.donate_url)
            xml.writeEndElement()

        xml.writeStartElement("devices")
        for dev_info in self.device_infos:
            dev_info.write_xml(xml)
        xml.writeEndElement()

        xml.writeStartElement("pit")
        last_slash = self.pit_filename.rfind('/')
        if last_slash < 0:
            last_slash = self.pit_filename.rfind('\\')

        xml.writeCharacters(self.pit_filename[last_slash + 1:])
        xml.writeEndElement()

        xml.writeStartElement("repartition")
        xml.writeCharacters("1" if self.repartition else "0")
        xml.writeEndElement()

        xml.writeStartElement("noreboot")
        xml.writeCharacters("1" if self.no_reboot else "0")
        xml.writeEndElement()

        xml.writeStartElement("files")
        for file_info in self.file_infos:
            file_info.write_xml(xml, Packaging.clashless_filename(self.file_infos))
        xml.writeEndElement()

        xml.writeEndElement()
        xml.writeEndDocument()
