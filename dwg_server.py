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


@app.post('/layers')
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

    final_data = data.stdout.decode('latin-1').split()

    print(final_data, file=sys.stderr)

    return json(final_data)


@app.post('/image')
async def parse_image(request: Request):
    request_file = request.files.get('file')
    if not request_file:
        return json({'message': 'No file selected!', 'success': False})
    if not request_file.name.split('.')[-1] == 'dwg':
        return json({'message': 'Selected file is not an dwg!', 'success': False})

    await delete_file(request_file)
    await save_file(request_file)

    file_name = load_file_name(request_file)

    original_file_path = os.path.join(os.getcwd(), request_file.name)
    target_file_path = os.path.join(os.getcwd(), file_name) + '.svg'

    print(f'original path: {original_file_path}', file=sys.stderr)
    print(f'target file: {target_file_path}', file=sys.stderr)

    dwg_layers = request.form.get("layers")
    print(f'layers: {dwg_layers}', file=sys.stderr)

    core_layer = request.form.get("core_layer")
    if not core_layer:
        core_layer = '080202_BEAUGEB_AWAND'  # defualt layer for boundaries

    if not dwg_layers:
        execute = f'dwg2SVG -b {core_layer} {original_file_path} >{target_file_path}'
    else:
        joined_layers = ":".join(dwg_layers.split(','))
        execute = f'dwg2SVG -b {core_layer} -l {joined_layers} {original_file_path} >{target_file_path}'

    print(f'execute: {execute}', file=sys.stderr)

    await delete_file_by_path(target_file_path)

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

    request_file_name = load_file_name(request_file)

    original_file_path = os.path.join(os.getcwd(), request_file.name)
    target_file_path = os.path.join(os.getcwd(), request_file_name) + '.csv'

    print(f'original path: {original_file_path}', file=sys.stderr)
    print(f'target file: {target_file_path}', file=sys.stderr)

    execute = f'dwgread_test {original_file_path} >{target_file_path}'

    print(f'execute: {execute}', file=sys.stderr)

    await delete_file_by_path(target_file_path)

    os.system(execute)

    return await file(target_file_path, filename=request_file_name + '.csv')


def load_file_name(in_file) -> str:
    split_name = in_file.name.split('.')
    split_name.pop()
    return ''.join(split_name)


async def save_file(file):
    temp_file = open(os.path.join(os.getcwd(), file.name), 'wb')
    temp_file.write(file.body)
    temp_file.close()


async def delete_file_by_path(file_path: str):
    if os.path.exists(file_path):
        os.remove(file_path)


async def delete_file(in_file):
    if os.path.exists(os.path.join(os.getcwd(), in_file.name)):
        os.remove(os.path.join(os.getcwd(), in_file.name))


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=args.port, fast=args.fast, workers=args.workers)

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
