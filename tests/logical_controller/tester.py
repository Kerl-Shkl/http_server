import subprocess
import requests
import time
import argparse

URL = "http://localhost:8678/home"
DATA = "some data"
TEST_KEY = "test_key"
TEST_VALUE = "test value"

server_process = None
args = None


def startServer():
    global server_process
    global args
    server_process = subprocess.Popen([args.executable])


def stopServer():
    global server_process
    server_process.terminate()


def sendRequest():
    return requests.request(
        method="get", url=URL, data=DATA, headers={TEST_KEY: TEST_VALUE}
    )


def checkResponse(response: requests.Response):
    if response.text != DATA:
        raise Exception("body isn't same")
    if TEST_KEY not in response.headers:
        raise Exception("No test header")
    if response.headers[TEST_KEY] != TEST_VALUE:
        raise Exception("different test value")


def main():
    global args

    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", help="full path to executable")
    args = parser.parse_args()

    startServer()
    time.sleep(0.5)
    response = sendRequest()
    try:
        checkResponse(response)
    except Exception as error:
        print(error)
        stopServer()
        exit(1)
    stopServer()


if __name__ == "__main__":
    main()
