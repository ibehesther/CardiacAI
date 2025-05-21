"""
This module is used to load the configuration settings from the .env file
"""

from dotenv import load_dotenv
import os

load_dotenv()


class Config:
    """
    This class is used to load the configuration settings from the .env file

    Attributes:
    - db: A dictionary containing the database configuration settings
    - secret_key: A string containing the secret key for the application
    """

    def __init__(self):
        # Why sould the default value be development?
        env = os.getenv("ENV", "dev")
        self._config = {
            "db": {
                "host": os.getenv("DB_HOST", "localhost"),
                "port": os.getenv("DB_PORT", "27017"),
            },
        }
        self._config["db"][
            "uri"
        ] = f"mongodb://{self.db['host']}:{self.db['port']}/"

        if env == "prod":
            self._config["db"]["name"] = os.getenv(
                "DB_NAME_PRODUCTION", "CARDIAC_prod"
            )
        elif env == "test":
            self._config["db"]["name"] = os.getenv("DB_NAME_TEST", "CARDIAC_test")
        else:
            self._config["db"]["name"] = os.getenv(
                "DB_NAME_DEVELOPMENT", "CARDIAC_dev"
            )
        self.secret_key = os.getenv("SECRET_KEY", "secret")
        self.token_algorithm = os.getenv("TOKEN_ALGORITHM", "HS256")
        self.token_expiration = int(os.getenv("TOKEN_EXPIRATION", 3600))


    @property
    def db(self):
        return self._config["db"]

    @property
    def SECRET_KEY(self):
        return self.secret_key

    @property
    def ALGORITHM(self):
        return self.token_algorithm

    @property
    def ACCESS_TOKEN_EXPIRE_SECONDS(self):
        return self.token_expiration


settings = Config()