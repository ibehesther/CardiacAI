import axios from "axios";

const REACT_APP_API_URL = process.env.REACT_APP_API_URL;

export const loginUser = async (username: string, password: string) => {
	const response = await axios.post(
		`${REACT_APP_API_URL}/api/auth/token`,
		{
			username,
			password,
			grant_type: "password",
			scope: "",
			client_id: "",
			client_secret: "",
		},
		{
			headers: {
				"Content-Type": "application/x-www-form-urlencoded",
				"Access-Control-Allow-Origin": "*",
			},
		}
	);

	return response.data;
};

export const getDeviceMetadata = async (deviceId: string, token: string) => {
	const response = await axios.get(
		`${REACT_APP_API_URL}/api/devices/metadata/${deviceId}`,
		{
			headers: {
				"Content-Type": "application/json",
				"Access-Control-Allow-Origin": "*",
				Authorization: `Bearer ${token}`,
			},
		}
	);

  console.log("Token Used: ", token);

  console.log("Device metadata response: ", response.data);

	return response.data;
};
