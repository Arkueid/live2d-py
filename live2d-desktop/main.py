from application import Application


def main():
    app = Application()

    app.load_config()

    app.setup()

    app.start()

    app.save_config()


if __name__ == "__main__":
    main()
