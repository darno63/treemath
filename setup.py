from distutils.core import setup, Extension

def main():
  setup(
    name="treemath",
    version="0.5",
    ext_modules = [
      Extension("treemath",["treemathmodule.c"])
    ]
  )

if __name__ == "__main__":
  main()
