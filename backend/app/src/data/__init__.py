"""
This file is used to connect to the MongoDB database.
"""

from motor import motor_asyncio
from pymongo import MongoClient
from config import settings
import asyncio

client = MongoClient(settings.db["uri"])

db = client[settings.db["name"]]
async_db = motor_asyncio.AsyncIOMotorClient(settings.db["uri"])[
    settings.db["name"]
]

print("✅ Connected to MongoDB")
async_db.get_io_loop = asyncio.get_running_loop