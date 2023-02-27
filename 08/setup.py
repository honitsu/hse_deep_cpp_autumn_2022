from setuptools import setup, Extension

def main():
    setup(
        name="utils",
        version="1.0.0",
        author="Egor Mushenko",
        ext_modules=[Extension("utils", ["utils.c"])]
    )

if __name__ == "__main__":
    main()
