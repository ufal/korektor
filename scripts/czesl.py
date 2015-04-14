import re
import xml.etree.ElementTree as ET

class CzeSLW:
    ns = {'wdata' : 'http://utkl.cuni.cz/czesl/'}

    def __init__(self, filename):
        self.wtree = ET.parse(filename)
        self.wroot = self.wtree.getroot()

    def get_token_by_id(self, id):
        token = self.wroot.find("./wdata:doc/wdata:para/wdata:w[@id='"+id+"']/wdata:token", CzeSLW.ns)
        return token.text

class CzeSLA:
    ns = {'ldata' : 'http://utkl.cuni.cz/czesl/'}

    def __init__(self, filename):
        self.atree = ET.parse(filename)
        self.aroot = self.atree.getroot()

    def set_wref(self, czesl_wref):
        """ Set the reference to w-file.
        :param czesl_wref: Object of type CzeSLW
        :return:
        """
        self.wref = czesl_wref

    def get_token_by_id(self, id):
        token = self.aroot.find("./ldata:doc/ldata:para/ldata:s/ldata:w[@id='"+id+"']/ldata:token", CzeSLA.ns)
        return token.text

    def determine_edge_path_format(self):
        """
        It seems that at least 2 formats in which the a-files are stored. Sometimes <edge> nodes found inside each sentence
         (/doc/para/s/w/edge) or they are stored at paragraph level (/doc/para/edge) .. <to> nodes are not explicitly
          stored if the format is /doc/para/s/w/edge since the <to> node is the <w> itself. In the case of /doc/para/edge
          format <to> nodes are explicitly stored. So, it is better to store this format for each file and use it when
          it is required. The method sets the self.edge_path_format to an integer value. The self.edge_path_format values
           and the meaning are given below.
            1 - /doc/para/edge
            2 - /doc/para/s/w/edge

        :return:
        """
        if self.aroot.findall(".//ldata:para/ldata:edge", CzeSLA.ns):
            self.edge_path_format = 1
        elif self.aroot.findall(".//ldata:para/ldata:s/ldata:w/ldata:edge", CzeSLA.ns):
            self.edge_path_format = 2

    def get_error_info_at_edge(self, id):
        """
        Method for finding whether the edge contains an error. If there is no error, the method returns False, otherwise
        the method returns a tuple with detailed information about the edge.

        :param id: Edge id
        :return: Whether the edge contains error, i.e., error info or mismatch of tokens at edges. Returns False if
        there is no error, the method returns a tuple of (w_tokens, a_tokens, error) at the given edge.
        """
        edge_error = False
        w_ids = []
        a_ids = []
        w_tokens = []
        a_tokens = []

        w_nodes = self.aroot.findall(".//ldata:edge[@id='"+id+"']/ldata:from", CzeSLA.ns)
        w_ids = map(lambda x: re.sub(r'w\#', '', x.text), w_nodes)

        if self.edge_path_format == 1:
            a_nodes = self.aroot.findall(".//ldata:edge[@id='"+id+"']/ldata:to", CzeSLA.ns)
            a_ids = map(lambda to:to.text, a_nodes)
        elif self.edge_path_format == 2:
            a_nodes = self.aroot.findall(".//ldata:para/ldata:s/ldata:w/ldata:edge[@id='"+id+"']/..", CzeSLA.ns)
            a_ids = map(lambda w:w.get('id'), a_nodes)

        w_tokens = map(lambda xid: self.wref.get_token_by_id(xid), w_ids)
        a_tokens = map(lambda xid: self.get_token_by_id(xid), a_ids)

        error_nodes = self.aroot.findall(".//ldata:edge[@id='"+id+"']/ldata:error/ldata:tag", CzeSLA.ns)
        if not error_nodes:
            # error without a tag, tokens mismatch between a-token and w-token
            if len(w_nodes) == 1 and len(a_nodes) == 1:
                if w_tokens[0] != a_tokens[0]:
                    return (id, w_ids, a_ids, w_tokens, a_tokens, ['multiple_choices'])
                else:
                    return edge_error
            else:
                return (id, w_ids, a_ids, w_tokens, a_tokens, ['unknown_error'])
        else:
            error_names = []
            error_names = map(lambda e: e.text, error_nodes)
            return (id, w_ids, a_ids, w_tokens, a_tokens, error_names)

    def print_errors(self):
        edges = self.aroot.findall(".//ldata:edge", CzeSLA.ns)
        edge_ids = map(lambda e: e.get('id'), edges)
        for eid in edge_ids:
            e = self.get_error_info_at_edge(eid)
            if e:
                print ' '.join(e[3]).ljust(25)+'\t'+' '.join(e[4]).ljust(25)+'\t'+\
                      '+'.join(e[5])+'\t'+e[0].ljust(10)+\
                      '\t'+' '.join(e[1]).ljust(10)+'\t'+' '.join(e[2])