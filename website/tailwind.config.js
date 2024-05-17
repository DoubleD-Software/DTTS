import { fontFamily } from "tailwindcss/defaultTheme";

/** @type {import('tailwindcss').Config} */
const config = {
	content: ["./src/**/*.{html,js,svelte,ts}"],
	theme: {
		extend: {
			colors: {
				'bg': '#121212',
				'bg-light': '#252525',
				'bg-lightest': '#2F2F2F',
				'select-gray': '#404040',
				'tx-gray': '#D0D0D0',
				'cf-green': '#05FF00',
				'warn-red': '#FF3D3D',
			},
			fontFamily: {
				'dg': ['digitalnumbers', ...fontFamily.sans],
			},
		}
	},
};

export default config;
