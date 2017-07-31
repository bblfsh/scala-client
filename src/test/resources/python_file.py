import os
import sys


class BblfshClient(object):
    """
    Babelfish gRPC client. Currently it is only capable of fetching UASTs.
    """

    def __init__(self, endpoint):
        """
        Initializes a new instance of BblfshClient.

        :param endpoint: The address of the Babelfish server, \
                         for example "0.0.0.0:9432"
        :type endpoint: str
        """

    def parse(self, filename, language=None, contents=None, timeout=None,
                   unicode_errors="ignore"):
        request = ParseRequest(filename=os.path.basename(filename),
                                   content=contents,
                                   language=language)
        response = self._stub.Parse(request, timeout=timeout)
        return response
