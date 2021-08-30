import requests
import argparse

parser = argparse.ArgumentParser(description='Generate restore height list.')
parser.add_argument('-P', '--port',
                    default='18081',
                    dest='port',
                    help='Daemon port',
                    type=int)

args = parser.parse_args()

DAEMON_ADDRESS = f"http://127.0.0.1:{args.port}"

def get_block_timestamp(height: int):
    data = {
        "jsonrpc": "2.0",
        "id": "0",
        "method": "get_block_header_by_height",
        "params": {
            "height": height
        }
    }
    res = requests.get(DAEMON_ADDRESS+"/json_rpc", json=data)
    return res.json()['result']['block_header']['timestamp']


def get_height():
    data = {
        "jsonrpc": "2.0",
        "id": "0",
        "method": "get_block_count"
    }
    res = requests.get(DAEMON_ADDRESS+"/json_rpc", json=data)
    return res.json()['result']['count']


timestamps = {}
current_height = get_height()

timestamps[1] = get_block_timestamp(1)
for height in range(1500, current_height, 1500):
    timestamps[height] = get_block_timestamp(height)

for val in timestamps:
    print(f"{timestamps[val]}:{val}")