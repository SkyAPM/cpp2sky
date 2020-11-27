import inspect
import os
import sys
import time
from difflib import Differ
from os.path import dirname

import yaml
import requests
from requests import Response

try:
  from yaml import CSafeLoader as Loader
except ImportError:
  from yaml import SafeLoader as Loader

def validate(expected_file_name):
  with open(expected_file_name) as expected_data_file:
    expected_data = os.linesep.join(expected_data_file.readlines())
    response = requests.post(url='http://0.0.0.0:12800/dataValidate', data=expected_data)

    if response.status_code != 200:
      res = requests.get('http://0.0.0.0:12800/receiveData')
      actual_data = yaml.dump(yaml.load(res.content, Loader=Loader))

      differ = Differ()
      diff_list = list(differ.compare(
        actual_data.splitlines(keepends=True),
        yaml.dump(yaml.load(expected_data, Loader=Loader)).splitlines(keepends=True)
      ))

      print('diff list: ')
      sys.stdout.writelines(diff_list)

    assert response.status_code == 200
    return response

if __name__ == "__main__":
  requests.get('http://0.0.0.0:8081/ping')
  validate('./data/expected.yaml')
