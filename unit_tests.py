"""This module includes a variety of unit tests to test the 42 webserv"""

import unittest
import requests

class WebservResponseCodeTests(unittest.TestCase):
    """Tests if webserv returns the the appropriate response codes."""
    def setUp(self):
        self.webserv_url = 'http://localhost:8080/'
    
    def test_basic_get(self):
        response = requests.get(self.webserv_url, timeout = 5)
        print(f"Basic GET test on {self.webserv_url}")
        self.assertEqual(response.status_code, 200)
    
    def test_missing_file(self):
        missing_file = self.webserv_url + "not_a_file.html"
        response = requests.get(missing_file)
        print("GET on missing file test")
        self.assertEqual(response.status_code, 404)

if __name__ == '__main__':
    unittest.main()