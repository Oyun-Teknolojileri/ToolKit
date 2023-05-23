import os
import subprocess

# Define the directory to start the search from
root_directory = './ToolKit/Editor'

# Define the file extensions to consider
file_extensions = ['.h', '.cpp']

# Define the license text to insert
license_text = '''/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
'''

# Recursively traverse through the directory structure
for root, dirs, files in os.walk(root_directory):
    for file in files:
        if file.endswith(tuple(file_extensions)):
            file_path = os.path.join(root, file)
            with open(file_path, 'r') as f:
                content = f.read()

            # Insert the license text at the beginning of the file
            modified_content = license_text + content

            # Write the modified content back to the file
            with open(file_path, 'w') as f:
                f.write(modified_content)
            
            # Run clang-format to format the file
            subprocess.run(['clang-format', '-i', file_path])