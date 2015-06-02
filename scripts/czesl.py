# -*- coding: utf-8 -*-
"""Module for extracting error data from CzeSL corpora.

"""

import codecs
import re
import xml.etree.ElementTree as ET
import sys

class CzeSLW:
    """Class representing the first level CzeSL data.

    """
    ns = {'wdata' : 'http://utkl.cuni.cz/czesl/'}

    def __init__(self, filename):
        self.wtree = ET.parse(filename)
        self.wroot = self.wtree.getroot()

    def get_token_by_id(self, id):
        try:
            token = self.wroot.find("./wdata:doc/wdata:para/wdata:w[@id='"+id+"']/wdata:token", CzeSLW.ns)
            return token.text
        except AttributeError:
            return '<TokenElementMissing>'

class CzeSLA:
    """Class representing the correction of orthographical errors in the first level CzeSL data.

    """

    ns = {'ldata' : 'http://utkl.cuni.cz/czesl/'}

    def __init__(self, filename):
        self.atree = ET.parse(filename)
        self.aroot = self.atree.getroot()

    def set_wref(self, czesl_wref):
        """ Set the reference to w-level data object.

        Args:
            czesl_wref: Object of type CzeSLW

        """
        self.wref = czesl_wref

    def get_token_by_id(self, id):
        try:
            token = self.aroot.find("./ldata:doc/ldata:para/ldata:s/ldata:w[@id='"+id+"']/ldata:token", CzeSLA.ns)
            return token.text
        except AttributeError:
            return '<TokenElementMissing>'

    def determine_edge_path_format(self):
        """Determine the format of the path to edge element in the xml file.

        It seems that at least 2 formats in which the a-files are stored. Sometimes <edge> nodes found inside
        each sentence (/doc/para/s/w/edge) or they are stored at paragraph level (/doc/para/edge) .. <to>
        nodes are not explicitly stored if the format is /doc/para/s/w/edge since the <to> node is the <w> itself.
        In the case of /doc/para/edge format <to> nodes are explicitly stored.
        So, it is better to store this format for each file and use it when it is required. The method sets the
        self.edge_path_format to an integer value. The self.edge_path_format values and the meaning are given below.
        - /doc/para/edge
        - /doc/para/s/w/edge

        """
        if self.aroot.findall(".//ldata:para/ldata:edge", CzeSLA.ns):
            self.edge_path_format = 1
        elif self.aroot.findall(".//ldata:para/ldata:s/ldata:w/ldata:edge", CzeSLA.ns):
            self.edge_path_format = 2

    def get_error_info_at_edge(self, edge_id):
        """Determine if the edge has an error in the upper level and return relevant details.

        Method for finding whether the edge contains an error. If there is no error, the method returns False,
        otherwise the method returns a tuple with detailed information about the edge.

        Args:
            id: Edge id

        Returns:
            Whether the edge contains error, i.e., error info or mismatch of tokens at edges. Returns False if
            there is no error, the method returns a tuple of (w_tokens, a_tokens, error) at the given edge.

        """
        edge_error = False
        w_ids = []
        a_ids = []
        w_tokens = []
        a_tokens = []

        w_ids = self.find_lower_level_node_ids(edge_id)
        if not w_ids:
            return edge_error
        w_tokens = map(lambda xid: self.wref.get_token_by_id(xid), w_ids)

        a_ids = self.find_current_level_node_ids(edge_id)
        a_tokens = map(lambda xid: self.get_token_by_id(xid), a_ids)

        # check if any token is empty, i.e., <token />
        # Warning: this error may be left out in the error outputs. It must be
        # handled some other way.
        for a_tok in a_tokens:
            if not a_tok:
                return False

        for w_tok in w_tokens:
            if not w_tok:
                return False

        # allow only 1-1 errors
        if len(a_tokens) !=1 or len(w_tokens) != 1:
            return False

        error_names = self.find_error_tags(edge_id)
        if not error_names:
            # error without a tag, tokens mismatch between a-token and w-token
            if len(w_ids) == 1 and len(a_ids) == 1:
                if w_tokens[0] != a_tokens[0]:
                    return (edge_id, w_ids, a_ids, w_tokens, a_tokens, ['multiple_choices'])
                else:
                    return edge_error
            else:
                return (edge_id, w_ids, a_ids, w_tokens, a_tokens, ['unknown_error'])
        else:
            error_names.sort()
            return (edge_id, w_ids, a_ids, w_tokens, a_tokens, error_names)

    def print_errors(self):
        edges = self.aroot.findall(".//ldata:edge", CzeSLA.ns)
        edge_ids = map(lambda e: e.get('id'), edges)
        for eid in edge_ids:
            e = self.get_error_info_at_edge(eid)
            if e:
                print ' '.join(e[3]).ljust(25)+'\t'+' '.join(e[4]).ljust(25)+'\t'+\
                      '+'.join(e[5])+'\t'+e[0].ljust(10)+\
                      '\t'+' '.join(e[1]).ljust(10)+'\t'+' '.join(e[2])

    def write_errors(self, file_prefix):
        out_file = file_prefix + '.err.txt'
        edges = self.aroot.findall(".//ldata:edge", CzeSLA.ns)
        edge_ids = map(lambda e: e.get('id'), edges)
        with codecs.open(out_file, mode='w', encoding='utf-8') as f:
            for eid in edge_ids:
                e = self.get_error_info_at_edge(eid)
                if e:
                    f.write(' '.join(e[3]).ljust(25)+'\t'+' '.join(e[4]).ljust(25)+'\t'+\
                          '+'.join(e[5])+'\t'+e[0].ljust(10)+\
                          '\t'+' '.join(e[1]).ljust(10)+'\t'+' '.join(e[2])+'\n')
        return

    def write_errors_by_sentences(self, file_prefix):
        """Write original sentences along with correct words if there are any spelling errors found.

        The sentence with errors will be printed if and only if,
        - There's a one to one correspondence with error word and the corrected word.
        - Any error word in the sentence is not multiple token represented with { } or the error word is not
          exactly an error. Examples:
            - w-word ->  u{ }čitelka,  a-word -> učitelka
            - w-word ->  a|o{ }dem, a-word -> a jdeme
        - All error tokens in the sentence must map to exactly one token each in the original sentence.

        Args:
            file_prefix: File name - only the prefix.

        """
        train_file = file_prefix + '.train.sen.txt'
        err_file = file_prefix + '.err.sen.txt'
        gold_file = file_prefix + '.gold.sen.txt'

        edges = self.aroot.findall(".//ldata:edge", CzeSLA.ns)
        edge_ids = map(lambda e: e.get('id'), edges)
        error_dictionary = {}
        for eid in edge_ids:
            e = self.get_error_info_at_edge(eid)
            if e:
                if len(e[1]) == 1 and len(e[2]) == 1 and e[5][0] != 'multiple_choices':
                    if not re.match(r'(\{\s+\}|\|)', e[3][0]):
                        error_dictionary[e[2][0]] = e

        # iterate over sentences
        with codecs.open(train_file, mode='w', encoding='utf-8') as f, codecs.open(gold_file, mode='w', encoding='utf-8') as gf, codecs.open(err_file, mode='w', encoding='utf-8') as ef:
            for sen in self.aroot.findall("./ldata:doc/ldata:para/ldata:s", CzeSLA.ns):
                tokens = []
                gtokens = []
                err_tokens = []
                for w in sen.findall("./ldata:w", CzeSLA.ns):
                    if not w.get('id') in error_dictionary:
                        t = w.find("./ldata:token", CzeSLA.ns).text
                        if t:
                            tokens.append(t)
                            gtokens.append(t)
                            err_tokens.append(t)
                    else:
                        gold_token = w.find("./ldata:token", CzeSLA.ns).text
                        orig_token = error_dictionary[w.get('id')][3][0]
                        error_signature = get_error_signature(orig_token, gold_token)
                        if error_signature:
                            tokens.append("<type=\""+error_signature+"\" " +"orig=\""+orig_token+"\" "+"gold=\""+gold_token+"\">")
                            err_tokens.append(orig_token)
                        else:
                            tokens.append(gold_token)
                            err_tokens.append(gold_token)
                        gtokens.append(gold_token)
                sen_to_write = ' '.join(tokens)
                gold_sen_to_write = ' '.join(gtokens)
                err_sen_to_write = ' '.join(err_tokens)
                f.write(sen_to_write + '\n')
                gf.write(gold_sen_to_write + '\n')
                ef.write(err_sen_to_write + '\n')
        return

    def find_current_level_node_ids(self, edge_id):
        current_ids = []
        if self.aroot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:to', CzeSLA.ns):
            current_nodes = self.aroot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:to', CzeSLA.ns)
            current_ids = map(lambda x: x.text, current_nodes)
        elif self.aroot.findall('.//ldata:edge[@id="'+edge_id+'"]/..', CzeSLA.ns):
            current_nodes = self.aroot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:w', CzeSLA.ns)
            current_ids = map(lambda x: x.get('id'), current_nodes)
        return current_ids

    def find_lower_level_node_ids(self, edge_id):
        next_level_ids = []
        if self.aroot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:from', CzeSLA.ns):
            next_level_nodes = self.aroot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:from', CzeSLA.ns)
            next_level_ids = map(lambda x: re.sub(r'w#', r'',x.text), next_level_nodes)
        return next_level_ids

    def find_error_tags(self, edge_id):
        error_tags = []
        if self.aroot.findall(".//ldata:edge[@id='"+edge_id+"']/ldata:error/ldata:tag", CzeSLA.ns):
            error_nodes = self.aroot.findall(".//ldata:edge[@id='"+edge_id+"']/ldata:error/ldata:tag", CzeSLA.ns)
            error_tags = map(lambda x: x.text, error_nodes)
        return error_tags

class CzeSLB:
    """Class representing the correction of grammar errors at level-a.

    """
    ns = {'ldata' : 'http://utkl.cuni.cz/czesl/'}

    def __init__(self, filename):
        self.btree = ET.parse(filename)
        self.broot = self.btree.getroot()

    def set_aref(self, czesl_aref):
        """ Set the reference to a-file data object.

        Args:
            czesl_aref: Object of type CzeSLA

        """
        self.aref = czesl_aref

    def get_token_by_id(self, id):
        try:
            token = self.broot.find('./ldata:doc/ldata:para/ldata:s/ldata:w[@id="'+id+'"]/ldata:token', CzeSLB.ns)
            return token.text
        except AttributeError:
            return '<TokenElementMissing>'

    def get_lower_level_word_id(self, bid):
        """Get a-level word element id given b-level word element id

        """
        from_node = self.broot.find('./ldata:doc/ldata:para/ldata:s/ldata:w[@id="'+bid+'"]/ldata:edge/ldata:from', CzeSLB.ns)
        a_level_id = re.sub(r'a#', r'', from_node.text)
        return a_level_id

    def get_lower_level_edge_id(self, edge_id):
        """Get an edge between a-level word element and w-level word element given the edge id between b-level word element and a-level word element.

        """
        # get 'word'/'token' elements in a-level if any
        if self.broot.findall(".//ldata:edge[@id='"+edge_id+"']/ldata:from", CzeSLB.ns):
            a_nodes = self.broot.findall(".//ldata:edge[@id='"+edge_id+"']/ldata:from", CzeSLB.ns)
            a_ids = map(lambda x: re.sub(r'a#', r'',x.text), a_nodes)
        else:
            return False

        # expected number of ids : 1
        if len(a_ids) == 1:
            a_edge_nodes = self.aref.aroot.findall('./ldata:doc/ldata:para/ldata:s/ldata:w[@id="'+a_ids[0]+'"]/ldata:edge', CzeSLA.ns)
            if len(a_edge_nodes) == 1:
                a_edge_id = a_edge_nodes[0].get('id')
                return a_edge_id
            if not a_edge_nodes or len(a_edge_nodes) > 1:
                return False
        else:
            return False



    def get_error_info_at_edge(self, edge_id):
        """Provide error info at an edge between b-level node and a-level node.

        """
        edge_error = False
        b_ids = []
        a_ids = []
        b_tokens = []
        a_tokens = []
        error_nodes = []
        error_names = []

        # get 'word'/'token' elements in b-level
        b_ids = self.find_current_level_node_ids(edge_id)
        b_tokens = map(lambda xid: self.get_token_by_id(xid), b_ids)

        # get 'word'/'token' elements in a-level if any

        a_ids = self.find_lower_level_node_ids(edge_id)
        if not a_ids:
            return edge_error
        a_tokens = map(lambda xid: self.aref.get_token_by_id(xid), a_ids)

        # get b-level edge errors
        if self.broot.findall(".//ldata:edge[@id='"+edge_id+"']/ldata:error/ldata:tag", CzeSLB.ns):
            error_nodes = self.broot.findall(".//ldata:edge[@id='"+edge_id+"']/ldata:error/ldata:tag", CzeSLB.ns)
            error_names = map(lambda x: x.text, error_nodes)
            error_names.sort()

        if len(b_tokens) ==1 and len(a_tokens) ==1:
            if b_tokens[0] == a_tokens[0] and not error_names:
                return False
            elif b_tokens[0] == a_tokens[0] and error_names:
                return (edge_id, a_ids, b_ids, a_tokens, b_tokens, error_names)
            elif b_tokens[0] != a_tokens[0]:
                return (edge_id, a_ids, b_ids, a_tokens, b_tokens, error_names)
        return edge_error

    def find_current_level_node_ids(self, edge_id):
        current_ids = []
        if self.broot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:to', CzeSLB.ns):
            current_nodes = self.broot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:to', CzeSLB.ns)
            current_ids = map(lambda x: x.text, current_nodes)
        elif self.broot.findall('.//ldata:edge[@id="'+edge_id+'"]/..', CzeSLB.ns):
            current_nodes = self.broot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:w', CzeSLB.ns)
            current_ids = map(lambda x: x.get('id'), current_nodes)
        return current_ids

    def find_lower_level_node_ids(self, edge_id):
        next_level_ids = []
        if self.broot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:from', CzeSLB.ns):
            next_level_nodes = self.broot.findall('.//ldata:edge[@id="'+edge_id+'"]/ldata:from', CzeSLB.ns)
            next_level_ids = map(lambda x: re.sub(r'a#', r'',x.text), next_level_nodes)
        return next_level_ids

    def find_error_tags(self, edge_id):
        error_tags = []
        if self.broot.findall(".//ldata:edge[@id='"+edge_id+"']/ldata:error/ldata:tag", CzeSLB.ns):
            error_nodes = self.broot.findall(".//ldata:edge[@id='"+edge_id+"']/ldata:error/ldata:tag", CzeSLB.ns)
            error_tags = map(lambda x: x.text, error_nodes)
        return error_tags

    def get_projected_error(self, edge_id):
        """Obtain error information between w-level and b-level node given the b-level edge id.

        """
        projected_error = False

        b_a_level_error = []
        a_w_level_error =[]

        b_a_level_error = self.get_error_info_at_edge(edge_id)
        lower_edge_id = self.get_lower_level_edge_id(edge_id)
        if lower_edge_id:
            a_w_level_error = self.aref.get_error_info_at_edge(lower_edge_id)

        if b_a_level_error:
            print 'b_tokens: ' + ','.join(b_a_level_error[4]) + '\t|\t' + 'a_tokens: ' + ','.join(b_a_level_error[3]) + '\t|\t' + 'errors: ' + ','.join(b_a_level_error[5])
        if a_w_level_error:
            print 'a_tokens: ' + ','.join(a_w_level_error[4]) + '\t|\t' + 'w_tokens: ' + ','.join(a_w_level_error[3]) + '\t|\t' + 'errors: ' + ','.join(a_w_level_error[5])

    def write_errors_by_sentences(self, file_prefix):
        return



def get_error_signature(misspelled, correct):
    """Get the spelling error type given the correct word and the misspelled word.

    The function imitates the functionality of the C++ header from src/create/create_error_model/get_error_signature.h

    Args:
        misspelled: Token with spelling error
        correct: Gold token

    Returns:
        The error signature if and only if the error is caused by only one of edit operations (insertion, deletion,
        substitution, or swapping).

    """

    signature = ''

    if len(misspelled) == len(correct):
        for i in range(len(misspelled)):
            if misspelled[i] != correct[i] and (i + 1) < len(misspelled) and misspelled[i+1] == correct[i] and misspelled[i] == correct[i+1]:
                for j in range(i+2, len(misspelled)):
                    if misspelled[j] != correct[j]:
                        return False
                signature = u'swap_'+ misspelled[i] + misspelled[i+1]
                return signature
            elif misspelled[i] != correct[i]:
                for j in range(i+1, len(misspelled)):
                    if misspelled[j] != correct[j]:
                        return False
                signature = u's_'+ misspelled[i] + correct[i]
                return signature
    elif len(misspelled) == len(correct)+1:
        for i in range(len(correct)):
            if misspelled[i] != correct[i]:
                for j in range(i, len(correct)):
                    if misspelled[j+1] != correct[j]:
                        return False

                signature = u'i_'+ misspelled[i]
                if i == 0:
                    signature += '^'
                else:
                    signature += correct[i-1]

                signature += correct[i]
                return signature

        signature = u'i_'+ misspelled[len(misspelled)-1]
        signature += correct[len(correct)-1]
        signature += '$'
        return signature
    elif len(misspelled)+1 == len(correct):
        for i in range(len(misspelled)):
            if misspelled[i] != correct[i]:
                for j in range(i, len(misspelled)):
                    if misspelled[j] != correct[j+1]:
                        return False
                signature = u'd_'+correct[i]
                if i == 0:
                    signature += '^'
                else:
                    signature += correct[i-1]
                return signature
        signature = u'd_'+correct[len(correct)-1]+correct[len(correct)-2]
        return signature
    return