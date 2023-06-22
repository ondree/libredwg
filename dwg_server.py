import argparse
import os
import subprocess
import sys

from sanic import Request, text, json, file
from sanic import Sanic


def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.


parser = argparse.ArgumentParser(description='Server for dwg parsing')
parser.add_argument('-p', dest='port', default=5025, help='port number', type=int)
parser.add_argument('--fast', dest='fast', default=False, help='Fast option')
parser.add_argument('-w', dest='workers', default=1, help='Number of workers', type=int)

app = Sanic(name='Parse_DWG')
args = parser.parse_args()


@app.get('/ping')
async def ping(request: Request):
    return text('Project for parsing dwg, version:(1.0)')


@app.post('layers')
async def layers(request: Request):
    request_file = request.files.get('file')
    if not request_file:
        return json({'message': 'No file selected!', 'success': False})
    if not request_file.name.split('.')[-1] == 'dwg':
        return json({'message': 'Selected file is not an dwg!', 'success': False})

    await delete_file(request_file)
    await save_file(request_file)

    original_file_path = os.path.join(os.getcwd(), request_file.name)

    data = subprocess.run(f'dwglayers {original_file_path}', capture_output=True, shell=True)
    print(data.stdout, file=sys.stderr)

    final_data = data.stdout.decode('utf-8').split()

    print(final_data, file=sys.stderr)

    return json(final_data)


@app.post('/image')
async def parse_image(request: Request):
    request_file = request.files.get('file')
    if not request_file:
        return json({'message': 'No file selected!', 'success': False})
    if not request_file.name.split('.')[-1] == 'dwg':
        return json({'message': 'Selected file is not an dwg!', 'success': False})

    await save_file(request_file)

    split_name = request_file.name.split('.')
    split_name.pop()
    file_name = ''.join(split_name)

    original_file_path = os.path.join(os.getcwd(), request_file.name)
    target_file_path = os.path.join(os.getcwd(), file_name) + '.svg'

    print(f'original path: {original_file_path}', file=sys.stderr)
    print(f'target file: {target_file_path}', file=sys.stderr)

    print(f'layers: {request.json.get("layers")}', file=sys.stderr)

    execute = 'dwg2SVG ' + original_file_path + ' >' + target_file_path + ' ' + request.json.get('layers')
    print(f'execute: {execute}', file=sys.stderr)
    os.system(execute)

    return await file(target_file_path, filename=file_name + '.svg')


@app.post('/parse')
async def parse_file(request: Request):
    request_file = request.files.get('file')
    if not request_file:
        return json({'message': 'No file selected!', 'success': False})
    if not request_file.name.split('.')[-1] == 'dwg':
        return json({'message': 'Selected file is not an dwg!', 'success': False})

    await save_file(request_file)

    name_of_file = request_file.name.split('.')
    name_of_file.pop()

    os.system('./dwgread_test ' + os.path.join(os.getcwd(), request_file.name))
    # os.system('./dwg2SVG ' + os.path.join(os.getcwd(), file.name) + '>' + name_of_file + '.svg')
    return text('Everything is awesome')


async def save_file(file):
    temp_file = open(os.path.join(os.getcwd(), file.name), 'wb')
    temp_file.write(file.body)
    temp_file.close()


async def delete_file(file):
    if os.path.exists(os.path.join(os.getcwd(), file.name)):
        os.remove(os.path.join(os.getcwd(), file.name))


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=args.port, fast=args.fast, workers=args.workers)

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
